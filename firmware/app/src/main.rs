#![no_std]
#![no_main]
#![allow(dead_code)]
#![allow(unused_imports)]
use assign_resources::assign_resources;
use embassy_stm32::Peri;

use core::iter::IntoIterator;
use core::ops::DerefMut;
use cortex_m_rt::entry;
use panic_probe as _;
#[cfg(feature = "defmt")]
use {defmt::*, defmt_rtt as _};

use embassy_executor::{Executor, InterruptExecutor};
use embassy_futures::select::{select, select3, Either, Either3};
use embassy_stm32::adc::{Adc, AdcChannel, AnyAdcChannel, SampleTime};
use embassy_stm32::dma::WritableRingBuffer;
use embassy_stm32::exti::ExtiInput;
use embassy_stm32::gpio::{Level, Output, OutputOpenDrain, Pull, Speed};
use embassy_stm32::interrupt::{InterruptExt, Priority};
use embassy_stm32::peripherals::{ADC1, DMA1, DMA1_CH1};
use embassy_stm32::Config;
use embassy_stm32::{interrupt, peripherals};
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::channel::{Channel, Sender};
use embassy_sync::mutex;
use embassy_time::{Duration, Ticker, Timer};
use heapless::Vec;
use static_cell::StaticCell;
mod dfu;
mod solderotto;
use solderotto::control::*;
use solderotto::*;

enum DebugState {
    Toggle,
}

type TipAdcAsyncMutex = mutex::Mutex<CriticalSectionRawMutex, TipAdc<'static, ADC1, DMA1_CH1>>;
type DebugChannel = Channel<CriticalSectionRawMutex, DebugState, 64>;

static EXECUTOR_HI: InterruptExecutor = InterruptExecutor::new();
static EXECUTOR_LOW: StaticCell<Executor> = StaticCell::new();

static CHANNEL: DebugChannel = Channel::new();

assign_resources! {
    dfu: DfuResources {
        dp: PA12,
        dm: PA11,
        usb: USB,
        flash: FLASH,
    }

    ext: SysResource {
        ext3: PC6,
        ext2: PC7,
        ext1: PC8,
        ext0: PC9,
    }

    zcd: ZcdResources {
        zcd: PC0,
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
        tip0: PC2,
        tip1: PC3,
    }

    tip_adc: TipAdcResources {
        adc: ADC1,
        dma: DMA1_CH1,
    }
}

#[inline]
fn calc_temp(adc: u16) -> f32 {
    let adc = adc as f32;
    4.30767839e-06 * adc * adc + 1.15134114e-01 * adc + 33.1575698
}

#[interrupt]
unsafe fn I2C1_EV() {
    EXECUTOR_HI.on_interrupt()
}

#[embassy_executor::task]
async fn zero_crossing(
    tip_adc: &'static TipAdcAsyncMutex,
    dbg_control: Sender<'static, CriticalSectionRawMutex, DebugState, 64>,
    zcd: ZcdResources,
    temp: TempResources,
    driver: OutputResource,
) {
    let mut read_buffer: [u16; 2] = [0; 2];
    let mut control = WaveControl::new(driver, 1000);

    let mut set_point = 2;
    let mut dur: Duration = Duration::default();

    // control.set_point(set_point);

    let mut zcd = ExtiInput::new(zcd.zcd, zcd.int, Pull::None);
    let mut tip0 = AdcChannel::degrade_adc(temp.tip0);
    let mut tip1 = AdcChannel::degrade_adc(temp.tip1);

    let mut foo = Ticker::every(Duration::from_secs(1));
    let mut bar = Ticker::every(Duration::from_secs(60));

    loop {
        match select3(zcd.wait_for_any_edge(), foo.next(), bar.next()).await {
            Either3::First(_) => match zcd.get_level() {
                Level::Low => {
                    let now = embassy_time::Instant::now();
                    {
                        dbg_control.send(DebugState::Toggle).await;
                        let mut locked_adc = tip_adc.lock().await;
                        let tip_adc = locked_adc.deref_mut();

                        tip_adc
                            .adc
                            .read(
                                tip_adc.dma.reborrow(),
                                [
                                    (&mut tip0, SampleTime::CYCLES247_5),
                                    (&mut tip1, SampleTime::CYCLES247_5),
                                ]
                                .into_iter(),
                                &mut read_buffer,
                            )
                            .await;
                    }
                    dur = embassy_time::Instant::now().duration_since(now);
                }
                Level::High => {
                    // control.drive_high(BridgeState::Load);
                    dbg_control.send(DebugState::Toggle).await;
                }
            },
            Either3::Second(_) => {
                info!("tip0: {:?}", calc_temp(read_buffer[0]));
                info!("tip1: {:?}", calc_temp(read_buffer[1]));
            }
            Either3::Third(_) => {
                // if set_point > 40 {
                //     set_point = 0;
                // }
                // set_point += 1;
                // control.set_point(set_point);

                info!("adjust setpoint to {}", set_point);
            }
        }
    }
}

#[embassy_executor::task]
async fn debug_task(ext: SysResource) {
    let mut dbg0 = OutputOpenDrain::new(ext.ext0, Level::Low, Speed::Low);
    // let mut dbg1 = OutputOpenDrain::new(ext.ext1, Level::Low, Speed::Low);
    // let mut dbg2 = OutputOpenDrain::new(ext.ext2, Level::Low, Speed::Low);
    // let mut dbg3 = OutputOpenDrain::new(ext.ext3, Level::Low, Speed::Low);

    loop {
        match CHANNEL.receive().await {
            DebugState::Toggle => {
                dbg0.toggle();
            }
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

    let sel1_a = Output::new(p.PF8, Level::Low, Speed::Low);
    let sel2_a = Output::new(p.PF10, Level::Low, Speed::Low);

    let sel1_b = Output::new(p.PF9, Level::Low, Speed::Low);
    let sel2_b = Output::new(p.PF0, Level::Low, Speed::Low);

    let spawner = EXECUTOR_HI.start(interrupt::I2C1_EV);
    spawner
        .spawn(zero_crossing(
            adc,
            CHANNEL.sender(),
            r.zcd,
            r.temp,
            r.driver,
        ))
        .unwrap();

    let executor = EXECUTOR_LOW.init(Executor::new());
    executor.run(|spawner| {
        // spawner.spawn(check_connection(adc)).unwrap();
        spawner.spawn(dfu::dfu(r.dfu)).unwrap();
        spawner.spawn(debug_task(r.ext)).unwrap();
    });
}
