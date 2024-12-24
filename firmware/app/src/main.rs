#![no_std]
#![no_main]

use assign_resources::assign_resources;

use core::iter::IntoIterator;
use core::ops::DerefMut;
use panic_probe as _;
#[cfg(feature = "defmt")]
use {defmt::*, defmt_rtt as _};

// use solderotto::{BridgeState, WaveControl};
use embassy_executor::{Executor, InterruptExecutor, Spawner};
use embassy_stm32::adc::{Adc, AdcChannel, AnyAdcChannel, SampleTime};
use embassy_stm32::exti::ExtiInput;
use embassy_stm32::gpio::{Level, OutputOpenDrain, Pull, Speed};
use embassy_stm32::peripherals;
use embassy_stm32::peripherals::{ADC1, DMA1, DMA1_CH1};
use embassy_stm32::usart::UartTx;
use embassy_stm32::usb;
use embassy_stm32::Config;
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::mutex;
use embassy_time::{Duration, Timer};
use static_cell::StaticCell;

type TipAdcAsyncMutex = mutex::Mutex<CriticalSectionRawMutex, TipAdc<'static, ADC1>>;
mod dfu;
mod solderotto;
use solderotto::TipAdc;

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
    }

    temp1: TempResources1 {
        tip1: PC2,
    }

    tip_adc: TipAdcResources {
        adc: ADC1,
        dma: DMA1_CH1,
    }
}

// #[embassy_executor::task]
// async fn zero_crossing(zcd: ZcdResources, driver: OutputResource, temp: TempResources) {
//     let mut zcd = ExtiInput::new(zcd.zcd, zcd.int, Pull::None);
//     let mut control = WaveControl::new(driver, 100);
//     control.set_point(0);
//
//     let mut read_buffer: [u16; 3] = [0; 3];
//     let mut adc = Adc::new(temp.adc);
//
//     let mut dma = temp.dma;
//     let mut vrefint = adc.enable_vrefint().degrade_adc();
//     let mut tip0 = AdcChannel::degrade_adc(temp.tip0);
//     let mut tip1 = AdcChannel::degrade_adc(temp.tip1);
//
//     loop {
//         zcd.wait_for_falling_edge().await;
//         control.drive_low();
//
//         let now = embassy_time::Instant::now();
//
//         adc.read(
//             &mut dma,
//             [
//                 (&mut vrefint, SampleTime::CYCLES247_5),
//                 (&mut tip0, SampleTime::CYCLES247_5),
//                 (&mut tip1, SampleTime::CYCLES247_5),
//             ]
//             .into_iter(),
//             &mut read_buffer,
//         )
//         .await;
//
//         let dur = embassy_time::Instant::now().duration_since(now);
//
//         let convert_to_millivolts = |sample: u16| {
//             const VREF_CALIB: u32 = 1648;
//             const VREF_CHARA: u32 = 3000;
//
//             ((u32::from(sample) * VREF_CHARA * VREF_CALIB) / (read_buffer[0] as u32 * 4095)) as u16
//         };
//
//         info!("vref: {}mV", adc.blocking_read(&mut vrefint));
//         info!("tio0: {}mV", convert_to_millivolts(read_buffer[1]));
//         info!("tip1: {}mV", convert_to_millivolts(read_buffer[2]));
//
//         zcd.wait_for_rising_edge().await;
//         control.drive_high(BridgeState::Com);
//     }
// }

#[embassy_executor::task]
async fn zero_crossing(tip_adc: &'static TipAdcAsyncMutex, zcd: ZcdResources, temp: TempResources) {
    let mut vref = tip_adc.lock().await.adc.enable_vrefint().degrade_adc();

    let mut read_buffer: [u16; 2] = [0; 2];

    let mut zcd = ExtiInput::new(zcd.zcd, zcd.int, Pull::None);
    let mut tip0 = AdcChannel::degrade_adc(temp.tip0);

    loop {
        zcd.wait_for_falling_edge().await;

        let now = embassy_time::Instant::now();

        {
            let mut locked_adc = tip_adc.lock().await;
            let tip_adc = locked_adc.deref_mut();

            tip_adc
                .adc
                .read(
                    &mut tip_adc.dma,
                    [
                        (&mut vref, SampleTime::CYCLES247_5),
                        (&mut tip0, SampleTime::CYCLES247_5),
                    ]
                    .into_iter(),
                    &mut read_buffer,
                )
                .await;
        }
        let dur = embassy_time::Instant::now().duration_since(now);

        // let convert_to_millivolts = |sample: u16| {
        //     const VREF_CALIB: u32 = 1648;
        //     const VREF_CHARA: u32 = 3000;
        //
        //     ((u32::from(sample) * VREF_CHARA * VREF_CALIB) / (read_buffer[0] as u32 * 4095)) as u16
        // };

        info!("vref: {}mV", read_buffer[0]);
        info!("tio0: {}mV", read_buffer[1]);
        // info!("tip1: {}mV", read_buffer[2]);

        zcd.wait_for_rising_edge().await;
    }
}

#[embassy_executor::task]
async fn check_connection(tip_adc: &'static TipAdcAsyncMutex, tips: TempResources1) {
    let mut vref = tip_adc.lock().await.adc.enable_vrefint().degrade_adc();

    let mut read_buffer: [u16; 2] = [0; 2];

    let mut tip1 = AdcChannel::degrade_adc(tips.tip1);
    loop {
        {
            let mut locked_adc = tip_adc.lock().await;
            let tip_adc = locked_adc.deref_mut();

            tip_adc
                .adc
                .read(
                    &mut tip_adc.dma,
                    [
                        (&mut vref, SampleTime::CYCLES247_5),
                        (&mut tip1, SampleTime::CYCLES247_5),
                    ]
                    .into_iter(),
                    &mut read_buffer,
                )
                .await;
        }
        info!("vref: {}", read_buffer[0]);
        info!("tip1: {}", read_buffer[1]);
    }
}

// #[embassy_executor::task]
// async fn external_pins(r: SysResource) {
//     let mut ext0 = OutputOpenDrain::new(r.ext0, Level::Low, Speed::Low);
//     let mut ext1 = OutputOpenDrain::new(r.ext1, Level::Low, Speed::Low);
//     let mut ext2 = OutputOpenDrain::new(r.ext2, Level::Low, Speed::Low);
//     let mut ext3 = OutputOpenDrain::new(r.ext3, Level::Low, Speed::Low);
//
//     loop {
//         ext0.toggle();
//         ext1.toggle();
//         ext2.toggle();
//         ext3.toggle();
//         Timer::after_millis(40).await;
//     }
// }

#[embassy_executor::main]
async fn main(spawner: Spawner) {
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

    spawner.must_spawn(zero_crossing(adc, r.zcd, r.temp));
    spawner.must_spawn(check_connection(adc, r.temp1));

    // spawner.spawn(dfu::dfu(r.dfu)).unwrap();
    // spawner
    //     .spawn(zero_crossing(r.zcd, r.driver, r.temp))
    //     .unwrap();
}
