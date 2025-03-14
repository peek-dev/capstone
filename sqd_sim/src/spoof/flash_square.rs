use std::ffi::c_void;

use crate::BaseType_t;

use super::led::Color;

// This may someday get functionality, but not today.
// And honestly I'd really like to change how the underlying driver works too.

#[no_mangle]
pub extern "C" fn xFlashSquare_Init() -> BaseType_t {
    1
}

#[no_mangle]
pub extern "C" fn xFlashSquare_Enable(
    _led_num: u8,
    _half_period_ticks: u16,
    _color: Color,
) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xFlashSquare_Disable(_led_num: u8) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xFlashSquare_DisableAll() -> BaseType_t {
    1
}

#[no_mangle]
pub extern "C" fn vFlashSquare_Thread(_arg0: *mut c_void) {}
