use std::fmt::Write;

use crate::event::{PieceType, Square};

/// To match the type defined in `led_translation.h`
#[repr(C)]
pub struct ZeroToTwoInts {
    len: u8,
    data: [u8; 2],
}

/// Define an LED translation that is somewhat easier to decode on the other end.
#[no_mangle]
pub extern "C" fn LEDTrans_Square(row: u8, col: u8) -> u8 {
    assert!(row < 8 && col < 8);
    row * 8 + col
}

#[no_mangle]
pub extern "C" fn LEDTrans_Ptype(p: PieceType) -> ZeroToTwoInts {
    assert!((p as u8) <= (PieceType::BlackPawn as u8));
    ZeroToTwoInts {
        len: if p == PieceType::EmptySquare { 0 } else { 1 },
        data: [64 + (p as u8), 0],
    }
}

pub fn lednum_to_string(n: u8) -> String {
    if n < 64 {
        Square {
            row: (n / 8) as usize,
            col: (n % 8) as usize,
        }
        .to_string()
    } else {
        let mut s = String::new();
        let pt: PieceType = (n - 64)
            .try_into()
            .expect("Invalid piece type into translation?");
        write!(&mut s, "{:?}", pt).unwrap();
        s
    }
}
