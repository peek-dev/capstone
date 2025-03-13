use std::ffi::c_void;

use crate::{event::PieceType, BaseType_t};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct BoardState {
    rows: [u32; 8],
}

pub type EmuBoardState = [[PieceType; 8]; 8];

impl Default for BoardState {
    fn default() -> Self {
        Self { rows: [0; 8] }
    }
}

impl From<EmuBoardState> for BoardState {
    fn from(value: EmuBoardState) -> Self {
        let mut bs = Self::default();
        for i in 0..value.len() {
            for j in 0..value[i].len() {
                assert!((value[i][j] as u32) <= 0xF);
                bs.rows[i] |= (value[i][j] as u32) << (4 * j);
            }
        }
        bs
    }
}

pub extern "C" fn xSensor_Init() -> BaseType_t {
    1
}
pub extern "C" fn vSensor_Thread(_arg0: *mut c_void) {}
// We will not support emulation of the calibration sensor mode at the moment.
