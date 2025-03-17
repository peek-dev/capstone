use std::ffi::c_void;

use crate::{
    event::{send_emu, EmuEvent, PieceType},
    BaseType_t,
};

/// To match the type defined in `led.h`
#[derive(Clone, Copy, Default, Debug)]
#[repr(C)]
pub struct Color {
    brightness: u8,
    blue: u8,
    green: u8,
    red: u8,
}
const MAX_BRIGHTNESS: u8 = 31;
const ACTUALLY_NORMALIZE: bool = false;

impl Color {
    pub fn red_norm(&self) -> u8 {
        assert!(self.brightness <= MAX_BRIGHTNESS);
        if ACTUALLY_NORMALIZE {
            ((self.brightness as usize * self.red as usize) / MAX_BRIGHTNESS as usize) as u8
        } else {
            self.red
        }
    }
    pub fn green_norm(&self) -> u8 {
        assert!(self.brightness <= MAX_BRIGHTNESS);
        if ACTUALLY_NORMALIZE {
            ((self.brightness as usize * self.green as usize) / MAX_BRIGHTNESS as usize) as u8
        } else {
            self.green
        }
    }
    pub fn blue_norm(&self) -> u8 {
        assert!(self.brightness <= MAX_BRIGHTNESS);
        if ACTUALLY_NORMALIZE {
            ((self.brightness as usize * self.blue as usize) / MAX_BRIGHTNESS as usize) as u8
        } else {
            self.blue
        }
    }
    pub fn is_on(&self) -> bool {
        self.brightness > 0 && (self.red > 0 || self.green > 0 || self.blue > 0)
    }
}

#[derive(Clone, Copy, Default, Debug)]
pub(crate) struct LEDState {
    pub board: [[Color; 8]; 8],
    pub white: [Color; PieceType::EmptySquare as usize],
    pub black: [Color; PieceType::EmptySquare as usize],
}

impl LEDState {
    pub fn get_pt(&self, pt: PieceType) -> Color {
        if pt < PieceType::EmptySquare {
            self.white[pt as usize]
        } else {
            self.black[PieceType::BlackPawn as usize - pt as usize]
        }
    }
}

pub const N_LED_SAVES: usize = 2;

#[derive(Debug)]
pub(crate) enum LedEvent {
    ClearBoard,
    SetColor(u8, Color),
    Commit,
    Save(u8),
    Restore(u8),
}

fn send_emu_led(event: LedEvent) {
    send_emu(EmuEvent::Led(event));
}

// The following functions intercept the C code's messages to the LED driver.

#[no_mangle]
pub extern "C" fn xLED_clear_board() -> BaseType_t {
    send_emu_led(LedEvent::ClearBoard);
    1
}

/// # Safety
/// `pColor` must be a valid, non-null pointer to a Color.
#[no_mangle]
pub unsafe extern "C" fn xLED_set_color(num: u8, pColor: *const Color) -> BaseType_t {
    assert!((*pColor).brightness <= MAX_BRIGHTNESS);
    send_emu_led(LedEvent::SetColor(num, *pColor));
    1
}
#[no_mangle]
pub extern "C" fn xLED_commit() -> BaseType_t {
    send_emu_led(LedEvent::Commit);
    1
}
#[no_mangle]
pub extern "C" fn xLED_save(save_num: u8) -> BaseType_t {
    send_emu_led(LedEvent::Save(save_num));
    1
}
#[no_mangle]
pub extern "C" fn xLED_restore(save_num: u8) -> BaseType_t {
    send_emu_led(LedEvent::Restore(save_num));
    1
}

#[no_mangle]
pub extern "C" fn xLED_Init() -> BaseType_t {
    1
}
#[no_mangle]
pub extern "C" fn vLED_Thread(_arg0: *mut c_void) {}
