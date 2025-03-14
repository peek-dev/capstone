use std::ffi::c_void;

use crate::{event::PieceType, BaseType_t};

#[repr(u8)]
pub enum PTYPE {
    Null,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    NullAlt,
}

#[repr(u8)]
#[derive(PartialEq, Eq, Clone, Copy)]
pub enum BUTTON_EVENT {
    TurnSwitch,
    Restart,
    Hint,
    Undo,
}

pub type NormalMove = u32;
pub type UndoMove = u32;

// Soon™ this will hook together with the python stockfish wrapper from the RPi software.
// But for now all it does is print out whatever gets sent. Sorry :(

#[no_mangle]
pub extern "C" fn xUART_Init() -> BaseType_t {
    1
}

#[no_mangle]
pub extern "C" fn xUART_to_wire(_move: u32) -> BaseType_t {
    println!("UART M→R: {:#010x}", _move);
    1
}
#[no_mangle]
pub extern "C" fn xUART_EncodeEvent(button: BUTTON_EVENT, _move: NormalMove) -> BaseType_t {
    let mut request = button as u32;
    if button == BUTTON_EVENT::TurnSwitch {
        // This works, modulo a bunch of dontcares.
        request = (_move & (!0x3)) | (button as u32);
    }
    xUART_to_wire(request)
}
#[no_mangle]
pub extern "C" fn vUART_SendCalibration(
    _min: u16,
    _max: u16,
    _row: u8,
    _col: u8,
    _ptype: PieceType,
) {
}

#[no_mangle]
pub unsafe extern "C" fn xPtypeFromWire(pt_in: PTYPE, whiteToMove: BaseType_t) -> PieceType {
    let black_pt = match pt_in {
        PTYPE::Pawn => PieceType::BlackPawn,
        PTYPE::Knight => PieceType::BlackKnight,
        PTYPE::Bishop => PieceType::BlackBishop,
        PTYPE::Rook => PieceType::BlackRook,
        PTYPE::Queen => PieceType::BlackQueen,
        PTYPE::King => PieceType::BlackKing,
        _ => PieceType::EmptySquare,
    };
    if whiteToMove == 1 {
        black_pt.color_change()
    } else {
        black_pt
    }
}
#[no_mangle]
pub extern "C" fn xPtypeToWire(pt_in: PieceType) -> PTYPE {
    match pt_in {
        PieceType::BlackPawn | PieceType::WhitePawn => PTYPE::Pawn,
        PieceType::BlackBishop | PieceType::WhiteBishop => PTYPE::Bishop,
        PieceType::BlackRook | PieceType::WhiteRook => PTYPE::Rook,
        PieceType::BlackKnight | PieceType::WhiteKnight => PTYPE::Knight,
        PieceType::BlackKing | PieceType::WhiteKing => PTYPE::King,
        PieceType::BlackQueen | PieceType::WhiteQueen => PTYPE::Queen,
        PieceType::EmptySquare => PTYPE::NullAlt,
    }
}

#[no_mangle]
pub extern "C" fn vUART_Task(_arg0: *mut c_void) {}
