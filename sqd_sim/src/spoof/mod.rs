#![allow(non_snake_case)]

use std::ffi::c_void;

pub mod button;
pub mod freertos;
pub mod led;
/// Overrides functions defined in `led_translation.h`
pub mod led_translation;
pub mod sensor;

extern "C" {
    pub fn mainThread(arg0: *mut c_void);
}
