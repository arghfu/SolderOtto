#![allow(dead_code)]

use super::OutputResource;
#[cfg(feature = "defmt")]
use defmt::{trace, warn};
use embassy_stm32::adc::{Adc, Instance};
use embassy_stm32::gpio::{Level, Output, Speed};

pub struct TipAdc<'d, T: Instance, U> {
    pub adc: Adc<'d, T>,
    pub dma: U,
}

impl<'d, T: Instance, U> TipAdc<'d, T, U> {
    pub fn new(adc: T, dma: U) -> Self {
        Self {
            adc: Adc::new(adc),
            dma,
        }
    }
}

pub(crate) mod control;
pub(crate) mod handle;
