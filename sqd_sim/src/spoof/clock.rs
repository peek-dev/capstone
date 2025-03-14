use std::ffi::c_void;

use crate::BaseType_t;

use super::GameTurn;

// For now, this is all stubs. Nothing interesting to see here.
// Soonish, I might add clock emulation support.
// But maybe not. For some functionality, probably.
// But I expect the millisecond-grade timing will never be a feature.

#[no_mangle]
pub extern "C" fn xClock_Init() -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn vClock_Thread(_arg0: *mut c_void) {}

#[repr(u8)]
pub enum ClockState {
    Running,
    Paused,
    Off,
    Undo,
    NotStarted,
    StaticNumbers,
}

#[no_mangle]
pub extern "C" fn xClock_run_test(_seconds_per_test: u8) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xClock_set_turn(_turn: GameTurn) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xClock_set_state(_state: ClockState) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xClock_set_increment(_increment_ms: u32) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xClock_set_times(_times_ms: *const u32) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xClock_set_numbers(_numbers: *const u16) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xClock_set_both_numbers(_number: u16) -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn xClock_render_state() -> BaseType_t {
    1
}
