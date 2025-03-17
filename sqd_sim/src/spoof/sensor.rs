use std::ffi::c_void;

use crate::{
    event::{send_emu, EmuEvent, PieceType},
    BaseType_t,
};

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct BoardState {
    rows: [u32; 8],
}

pub type EmuBoardState = [[PieceType; 8]; 8];

pub const fn starting_board() -> EmuBoardState {
    [
        [
            PieceType::WhiteRook,
            PieceType::WhiteKnight,
            PieceType::WhiteBishop,
            PieceType::WhiteQueen,
            PieceType::WhiteKing,
            PieceType::WhiteBishop,
            PieceType::WhiteKnight,
            PieceType::WhiteRook,
        ],
        [PieceType::WhitePawn; 8],
        [PieceType::EmptySquare; 8],
        [PieceType::EmptySquare; 8],
        [PieceType::EmptySquare; 8],
        [PieceType::EmptySquare; 8],
        [PieceType::BlackPawn; 8],
        [
            PieceType::BlackRook,
            PieceType::BlackKnight,
            PieceType::BlackBishop,
            PieceType::BlackQueen,
            PieceType::BlackKing,
            PieceType::BlackBishop,
            PieceType::BlackKnight,
            PieceType::BlackRook,
        ],
    ]
}

impl From<EmuBoardState> for BoardState {
    fn from(value: EmuBoardState) -> Self {
        let mut bs = Self::default();
        for (i, row) in value.iter().enumerate() {
            for (j, piece) in row.iter().enumerate() {
                assert!((*piece as u32) <= 0xF);
                bs.rows[i] |= (*piece as u32) << (4 * j);
            }
        }
        bs
    }
}

#[no_mangle]
pub extern "C" fn xSensor_Init() -> BaseType_t {
    send_emu(EmuEvent::ResendSensor);
    1
}
#[no_mangle]
pub extern "C" fn vSensor_Thread(_arg0: *mut c_void) {}
// We will not support emulation of the calibration sensor mode at the moment.
