#![no_std]
#![no_main]
#![allow(dead_code)]
#![allow(unused_imports)]
use assign_resources::assign_resources;

use core::iter::IntoIterator;
use core::ops::DerefMut;
use cortex_m_rt::entry;
use panic_probe as _;
#[cfg(feature = "defmt")]
use {defmt::*, defmt_rtt as _};

use embassy_executor::{Executor, InterruptExecutor};
use embassy_futures::select::{select, Either4};
use embassy_stm32::adc::{Adc, AdcChannel, AnyAdcChannel, SampleTime};
use embassy_stm32::dma::WritableRingBuffer;
use embassy_stm32::exti::ExtiInput;
use embassy_stm32::gpio::{Level, OutputOpenDrain, Pull, Speed};
use embassy_stm32::interrupt::{InterruptExt, Priority};
use embassy_stm32::peripherals::{ADC1, DMA1, DMA1_CH1};
use embassy_stm32::Config;
use embassy_stm32::{interrupt, peripherals};
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::mutex;
use embassy_time::{Duration, Timer};
use heapless::Vec;
use static_cell::StaticCell;
mod dfu;
mod solderotto;
use solderotto::*;

type TipAdcAsyncMutex = mutex::Mutex<CriticalSectionRawMutex, TipAdc<'static, ADC1>>;

static EXECUTOR_HI: InterruptExecutor = InterruptExecutor::new();
static EXECUTOR_LOW: StaticCell<Executor> = StaticCell::new();

assign_resources! {
    dfu: DfuResources {
        dp: PA12,
        dm: PA11,
        usb: USB,
        flash: FLASH,
    }

    ext: SysResource {
        ext3: PA15,
        ext2: PC10,
        ext1: PC11,
        ext0: PC12,
    }

    zcd: ZcdResources {
        zcd: PB0,
        int: EXTI0,
    }

    driver: OutputResource {
        high_side_com: PC13,
        low_side_com: PF5,
        high_side_load0: PF3,
        high_side_load1: PF4,
        low_side_load: PF7,
    }

    temp: TempResources {
        tip0: PC3,
        tip1: PC2,
    }

    tip_adc: TipAdcResources {
        adc: ADC1,
        dma: DMA1_CH1,
    }
}

#[interrupt]
unsafe fn I2C1_EV() {
    EXECUTOR_HI.on_interrupt()
}

#[embassy_executor::task]
async fn zero_crossing(
    tip_adc: &'static TipAdcAsyncMutex,
    zcd: ZcdResources,
    temp: TempResources,
    driver: OutputResource,
) {
    // let mut vref = tip_adc.lock().await.adc.enable_vrefint().degrade_adc();

    let mut read_buffer: [u16; 3] = [0; 3];
    let mut control = WaveControl::new(driver, 1000);
    control.set_point(0);

    let mut zcd = ExtiInput::new(zcd.zcd, zcd.int, Pull::None);
    let mut tip0 = AdcChannel::degrade_adc(temp.tip0);
    // let mut tip1 = AdcChannel::degrade_adc(temp.tip1);

    loop {
        zcd.wait_for_falling_edge().await;
        control.drive_low();

        let now = embassy_time::Instant::now();

        match select(
            zcd.wait_for_falling_edge(),
            Timer::after(Duration::from_secs(8)),
            self.pin.wait_for_falling_edge(),
        )
        .await
        {
            Either4::First(_) => {
                if self.pin.is_high() {
                    cortex_m::peripheral::SCB::sys_reset();
                }
            }
            Either4::Second(_) => {}
        }

        {
            let mut locked_adc = tip_adc.lock().await;
            let tip_adc = locked_adc.deref_mut();

            tip_adc
                .adc
                .read(
                    &mut tip_adc.dma,
                    [(&mut tip0, SampleTime::CYCLES247_5)].into_iter(),
                    &mut read_buffer[0..1],
                )
                .await;
        }
        let dur = embassy_time::Instant::now().duration_since(now);

        info!("tip0: {}mV", read_buffer[0]);

        zcd.wait_for_rising_edge().await;
        control.drive_high(BridgeState::Com);
    }
}

#[embassy_executor::task]
async fn check_connection(tip_adc: &'static TipAdcAsyncMutex) {
    let mut vref = tip_adc.lock().await.adc.enable_vrefint().degrade_adc();

    let mut read_buffer: [u16; 8] = [0; 8];

    // let mut tip1 = AdcChannel::degrade_adc(tips.tip1);
    loop {
        {
            let mut locked_adc = tip_adc.lock().await;
            let tip_adc = locked_adc.deref_mut();

            tip_adc
                .adc
                .read(
                    &mut tip_adc.dma,
                    [(&mut vref, SampleTime::CYCLES247_5)].into_iter(),
                    &mut read_buffer[0..1],
                )
                .await;
        }
    }
}

#[entry]
fn main() -> ! {
    #[cfg(feature = "defmt")]
    info!("Hello, World!");

    let mut config = Config::default();
    {
        use embassy_stm32::rcc::*;
        config.rcc.hsi = true;
        config.rcc.pll = Some(Pll {
            source: PllSource::HSI,
            prediv: PllPreDiv::DIV1,
            mul: PllMul::MUL20,
            // Main system clock at 160 MHz
            divr: Some(PllRDiv::DIV2),
            divq: Some(PllQDiv::DIV8),
            divp: Some(PllPDiv::DIV2),
        });
        config.rcc.mux.adc12sel = mux::Adcsel::SYS;
        config.rcc.mux.adc345sel = mux::Adcsel::SYS;
        config.rcc.sys = Sysclk::PLL1_R;
    }

    let p = embassy_stm32::init(config);
    let r = split_resources!(p);

    static ADC: StaticCell<TipAdcAsyncMutex> = StaticCell::new();
    let adc = ADC.init(mutex::Mutex::new(TipAdc::new(r.tip_adc.adc, r.tip_adc.dma)));

    interrupt::I2C1_EV.set_priority(Priority::P4);
    let spawner = EXECUTOR_HI.start(interrupt::I2C1_EV);
    spawner
        .spawn(zero_crossing(adc, r.zcd, r.temp, r.driver))
        .unwrap();

    let executor = EXECUTOR_LOW.init(Executor::new());
    executor.run(|spawner| {
        // spawner.spawn(check_connection(adc)).unwrap();
        spawner.spawn(dfu::dfu(r.dfu)).unwrap()
    });
}
