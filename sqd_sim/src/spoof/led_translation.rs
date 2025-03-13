use crate::event::PieceType;

/// To match the type defined in `led_translation.h`
#[repr(C)]
pub struct ZeroToTwoInts {
    len: u8,
    data: [u8; 2],
}

/// Define an LED translation that is somewhat easier to decode on the other end.
pub extern "C" fn LEDTrans_Square(row: u8, col: u8) -> u8 {
    assert!(row < 8 && col < 8);
    row * 8 + col
}

pub extern "C" fn LEDTrans_Ptype(p: PieceType) -> ZeroToTwoInts {
    assert!((p as u8) <= (PieceType::BlackPawn as u8));
    ZeroToTwoInts {
        len: if p == PieceType::EmptySquare { 0 } else { 1 },
        data: [64 + (p as u8), 0],
    }
}
