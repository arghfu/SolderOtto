#![allow(dead_code)]

use super::OutputResource;
#[cfg(feature = "defmt")]
use defmt::{warn, trace};
use embassy_stm32::gpio::{Level, Output, Speed};

pub(crate) mod handle;

pub enum BridgeState {
    Load,
    Com,
}

pub struct WaveControl<'d> {
    driver: OutputStage<'d>,
    period: u16,
    set_point: u16,
    count: u16,
    enable: bool,
}

impl<'d> WaveControl<'d> {
    pub fn new(res: OutputResource, period: u16) -> Self {
        Self {
            driver: OutputStage::new(res),
            period,
            count: 0,
            set_point: 0,
            enable: true,
        }
    }

    pub fn drive_low(&mut self) {
        self.driver.set_low();
    }

    pub fn drive_high(&mut self, state: BridgeState) {
        if !self.enable {
            return;
        }

        if self.count >= self.period {
            self.count = 0;
        }

        if self.count < self.set_point {
            match state {
                BridgeState::Load => self.driver.set_load_high(),
                BridgeState::Com => self.driver.set_com_high(),
            }
        }

        self.count += 1;
    }

    pub fn set_point(&mut self, set_point: u16) {
        self.set_point = set_point;
    }
}

struct OutputStage<'d> {
    high_load0: Output<'d>,
    high_load1: Output<'d>,
    high_com: Output<'d>,
    low_load: Output<'d>,
    low_com: Output<'d>,
}

impl<'d> OutputStage<'d> {
    pub fn new(res: OutputResource) -> Self {
        Self {
            high_load0: Output::new(res.high_side_load0, Level::Low, Speed::Low),
            high_load1: Output::new(res.high_side_load1, Level::Low, Speed::Low),
            high_com: Output::new(res.high_side_com, Level::Low, Speed::Low),
            low_load: Output::new(res.low_side_load, Level::Low, Speed::Low),
            low_com: Output::new(res.low_side_com, Level::Low, Speed::Low),
        }
    }

    fn set_load_high(&mut self) {
        if !self.low_load.is_set_high() {
            self.high_load0.set_high();
            self.low_com.set_high();
            #[cfg(feature = "defmt")]
            trace! {"set load path high"}
        } else {
            #[cfg(feature = "defmt")]
            warn!("cannot drive load side!")
        }
    }

    fn set_com_high(&mut self) {
        if !self.low_com.is_set_high() {
            self.high_com.set_high();
            self.low_load.set_high();
            #[cfg(feature = "defmt")]
            trace! {"set com path high"}
        } else {
            #[cfg(feature = "defmt")]
            warn!("cannot drive com side!")
        }
    }

    fn set_load_low(&mut self) {
        self.high_load0.set_low();
        self.low_com.set_low();
    }

    fn set_com_low(&mut self) {
        self.high_com.set_low();
        self.low_load.set_low();
    }

    fn set_low(&mut self) {
        self.low_com.set_low();
        self.low_load.set_low();
        self.high_load0.set_low();
        // self.high_load1.set_low();
        self.high_com.set_low();
        #[cfg(feature = "defmt")]
        trace! {"set path low"}
    }
}
