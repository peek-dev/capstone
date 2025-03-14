#![allow(non_snake_case)]

use std::ffi::c_void;

pub mod button;
pub mod clock;
pub mod flash_square;
pub mod freertos;
pub mod led;
/// Overrides functions defined in `led_translation.h`
pub mod led_translation;
pub mod sensor;
pub mod uart;

#[repr(u8)]
pub enum GameTurn {
    White = 0,
    Black = 1,
    // Only used for signaling the clock.
    Over,
}

extern "C" {
    pub fn mainThread(arg0: *mut c_void);
}
