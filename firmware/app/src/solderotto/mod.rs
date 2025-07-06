#![allow(dead_code)]

use embassy_stm32::adc::{Adc, Instance};
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_stm32::peripherals::DMA1_CH1;
use embassy_stm32::Peri;

pub struct TipAdc<'d, T: Instance, U: embassy_stm32::PeripheralType> {
    pub adc: Adc<'d, T>,
    pub dma: Peri<'d, U>,
}

impl<'d, T: Instance, U: embassy_stm32::PeripheralType> TipAdc<'d, T, U> {
    pub fn new(adc: Peri<'static, T>, dma: Peri<'static, U>) -> Self {
        Self {
            adc: Adc::new(adc),
            dma,
        }
    }
}

pub(crate) mod control;
pub(crate) mod handle;
