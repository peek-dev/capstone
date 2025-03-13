use std::{ffi::c_void, sync::mpsc::Sender};

use lazy_static::lazy_static;
use parking_lot::Mutex;

use crate::{
    event::{send_emu, send_ui, EmuEvent, PieceType, UIEvent, UI_CHANNELS},
    BaseType_t,
};

/// To match the type defined in `led.h`
#[derive(Clone, Copy)]
#[repr(C)]
pub struct Color {
    brightness: u8,
    blue: u8,
    green: u8,
    red: u8,
}

impl Default for Color {
    fn default() -> Self {
        Self {
            brightness: 0,
            blue: 0,
            green: 0,
            red: 0,
        }
    }
}

#[derive(Clone, Copy, Default)]
pub(crate) struct LEDState {
    pub board: [[Color; 8]; 8],
    pub white: [Color; 6],
    pub black: [Color; 6],
}

const N_LED_SAVES: usize = 2;
lazy_static! {
    static ref LED_STATE: Mutex<LEDState> = Default::default();
    static ref SAVED_STATE: Mutex<[LEDState; N_LED_SAVES]> = Default::default();
}

pub(crate) enum LEDEvent {
    ClearBoard,
    SetColor(u8, Color),
    Commit,
    Save(u8),
    Restore(u8),
}

/// This function is only intended to be called by the core emulator thread.
pub(crate) fn inner_LED_DoEvent(event: LEDEvent) {
    match event {
        LEDEvent::ClearBoard => {
            *(LED_STATE.lock()) = LEDState::default();
        }
        LEDEvent::SetColor(num, color) => {
            // Note, this assumes that our nonstandard LED translation is overriding
            // the default translation layer.
            let mut state = LED_STATE.lock();
            if num < 64 {
                state.board[(num / 8) as usize][(num % 8) as usize] = color;
            } else {
                let p = num - 64;
                if p > PieceType::EmptySquare as u8 {
                    assert!(PieceType::BlackPawn as u8 - p < 6);
                    state.black[(PieceType::BlackPawn as u8 - p) as usize] = color;
                } else {
                    assert!(p < 6);
                    state.white[p as usize] = color;
                }
            }
        }
        LEDEvent::Commit => {
            send_ui(UIEvent::LEDChange(*LED_STATE.lock()));
        }
        LEDEvent::Save(save_num) => {
            assert!(save_num < N_LED_SAVES as u8);
            SAVED_STATE.lock()[save_num as usize] = *LED_STATE.lock();
        }
        LEDEvent::Restore(save_num) => {
            assert!(save_num < N_LED_SAVES as u8);
            *LED_STATE.lock() = SAVED_STATE.lock()[save_num as usize];
        }
    }
}

fn send_emu_led(event: LEDEvent) {
    send_emu(EmuEvent::LED(event));
}

// The following functions intercept the C code's messages to the LED driver.

pub extern "C" fn xLED_clear_board() -> BaseType_t {
    send_emu_led(LEDEvent::ClearBoard);
    1
}
pub unsafe extern "C" fn xLED_set_color(num: u8, pColor: *const Color) -> BaseType_t {
    send_emu_led(LEDEvent::SetColor(num, *pColor));
    1
}
pub extern "C" fn xLED_commit() -> BaseType_t {
    send_emu_led(LEDEvent::Commit);
    1
}
pub extern "C" fn xLED_save(save_num: u8) -> BaseType_t {
    send_emu_led(LEDEvent::Save(save_num));
    1
}
pub extern "C" fn xLED_restore(save_num: u8) -> BaseType_t {
    send_emu_led(LEDEvent::Restore(save_num));
    1
}

pub extern "C" fn xLED_Init() -> BaseType_t {
    1
}
pub extern "C" fn vLED_Thread(_arg0: *mut c_void) {}
