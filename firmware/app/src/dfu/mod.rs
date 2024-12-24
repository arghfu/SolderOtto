use super::DfuResources;
use core::cell::RefCell;
use embassy_boot_stm32::{AlignedBuffer, BlockingFirmwareState, FirmwareUpdaterConfig};
use embassy_stm32::flash::{Flash, WRITE_SIZE};
use embassy_stm32::usb::{self, Driver};
use embassy_stm32::{bind_interrupts, peripherals};
use embassy_sync::blocking_mutex::Mutex;
use embassy_time::Duration;
use embassy_usb::Builder;
use embassy_usb_dfu::{consts::DfuAttributes, usb_dfu, Control, ResetImmediate};

bind_interrupts!(struct Irqs {
    USB_LP => usb::InterruptHandler<peripherals::USB>;
});

#[embassy_executor::task]
pub async fn dfu(r: DfuResources) {
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

// struct Builder<'d, >
