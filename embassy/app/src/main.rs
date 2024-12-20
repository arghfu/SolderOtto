#![no_std]
#![no_main]

use assign_resources::assign_resources;
use core::any::Any;
use core::cell::RefCell;
use core::iter::IntoIterator;
#[cfg(feature = "defmt")]
use defmt::*;
#[cfg(feature = "defmt")]
use {defmt_rtt as _, panic_probe as _};

use embassy_boot_stm32::{AlignedBuffer, BlockingFirmwareState, FirmwareUpdaterConfig};
use embassy_executor::{Executor, InterruptExecutor, Spawner};
use embassy_stm32::adc::{Adc, AdcChannel, AnyAdcChannel, SampleTime};
use embassy_stm32::exti::ExtiInput;
use embassy_stm32::flash::{Flash, WRITE_SIZE};
use embassy_stm32::gpio::{Level, Output, OutputOpenDrain, Pull, Speed};
use embassy_stm32::spi::{Config as SpiConfig, Spi};
use embassy_stm32::time::Hertz;
use embassy_stm32::Config;
use embassy_stm32::{bind_interrupts, peripherals};
use embassy_sync::blocking_mutex::Mutex;
use embassy_time::{Duration, Timer};

use crate::solderotto::{BridgeState, WaveControl};
use embassy_stm32::usb::{self, Driver};
use embassy_usb::Builder;
use embassy_usb_dfu::{consts::DfuAttributes, usb_dfu, Control, ResetImmediate};

mod solderotto;

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
        dma: DMA1_CH1,
        adc: ADC1,
    }
}

bind_interrupts!(struct Irqs {
    USB_LP => usb::InterruptHandler<peripherals::USB>;
});

#[embassy_executor::task]
async fn zero_crossing(zcd: ZcdResources, driver: OutputResource, temp: TempResources) {
    let mut zcd = ExtiInput::new(zcd.zcd, zcd.int, Pull::None);
    let mut control = WaveControl::new(driver, 100);
    control.set_point(0);

    let mut read_buffer: [u16; 2] = [0, 2];
    let mut adc = Adc::new(temp.adc);

    let mut dma = temp.dma;
    let mut tip0 = AdcChannel::degrade_adc(temp.tip0);
    let mut tip1 = AdcChannel::degrade_adc(temp.tip1);

    loop {
        zcd.wait_for_falling_edge().await;
        control.drive_low();

        let now = embassy_time::Instant::now();

        adc.read(
            &mut dma,
            [
                (&mut tip0, SampleTime::CYCLES247_5),
                (&mut tip1, SampleTime::CYCLES247_5),
            ]
            .into_iter(),
            &mut read_buffer,
        )
        .await;

        let dur = embassy_time::Instant::now().duration_since(now);

        info!("time to read: {} us", dur.as_micros());
        debug!("tip0: {}, tip1: {}", read_buffer[0], read_buffer[1]);

        zcd.wait_for_rising_edge().await;
        control.drive_high(BridgeState::Com);
    }
}

#[embassy_executor::task]
async fn external_pins(r: SysResource) {
    let mut ext0 = OutputOpenDrain::new(r.ext0, Level::Low, Speed::Low);
    let mut ext1 = OutputOpenDrain::new(r.ext1, Level::Low, Speed::Low);
    let mut ext2 = OutputOpenDrain::new(r.ext2, Level::Low, Speed::Low);
    let mut ext3 = OutputOpenDrain::new(r.ext3, Level::Low, Speed::Low);

    loop {
        ext0.toggle();
        ext1.toggle();
        ext2.toggle();
        ext3.toggle();
        Timer::after_millis(40).await;
    }
}

#[embassy_executor::task]
async fn dfu(r: DfuResources) {
    let flash = Flash::new_blocking(r.flash);
    let flash = Mutex::new(RefCell::new(flash));

    let config = FirmwareUpdaterConfig::from_linkerfile_blocking(&flash, &flash);
    let mut magic = AlignedBuffer([0; WRITE_SIZE]);
    let mut firmware_state = BlockingFirmwareState::from_config(config, &mut magic.0);
    firmware_state.mark_booted().expect("Failed to mark booted");

    let driver = Driver::new(r.usb, Irqs, r.dp, r.dm);
    let mut config = embassy_usb::Config::new(0xc0de, 0xcafe);
    config.manufacturer = Some("FireWaterBurn");
    config.product = Some("USB-DFU Runtime example");
    config.serial_number = Some("08151337");

    let mut config_descriptor = [0; 256];
    let mut bos_descriptor = [0; 256];
    let mut control_buf = [0; 64];
    let mut state = Control::new(firmware_state, DfuAttributes::CAN_DOWNLOAD);
    let mut builder = Builder::new(
        driver,
        config,
        &mut config_descriptor,
        &mut bos_descriptor,
        &mut [],
        &mut control_buf,
    );

    usb_dfu::<_, _, ResetImmediate>(&mut builder, &mut state, Duration::from_millis(2500));

    let mut dev = builder.build();

    loop {
        dev.run().await
    }
}
#[embassy_executor::main]
async fn main(spawner: Spawner) {
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

    spawner.spawn(dfu(r.dfu)).unwrap();
    spawner.spawn(external_pins(r.ext)).unwrap();
    spawner
        .spawn(zero_crossing(r.zcd, r.driver, r.temp))
        .unwrap();
}
