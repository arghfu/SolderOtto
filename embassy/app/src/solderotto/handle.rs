use embassy_stm32::adc::{AdcChannel, AnyAdcChannel};
use embassy_stm32::peripherals::ADC1;

pub struct Handle {
    pub handle_type: Option<HandleType>,

}

impl Handle {
    pub fn new() -> Handle {
        
        Self { 
            handle_type: None,
        }
    }
}


#[allow(unused)]
pub enum HandleType {
    T210,
    T245,
    AM120,
}


