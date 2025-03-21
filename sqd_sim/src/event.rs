use std::{
    fmt::Display,
    sync::mpsc::{channel, Receiver, Sender},
};

use once_cell::sync::Lazy;
use parking_lot::Mutex;

use crate::{
    spoof::button::ButtonNum,
    spoof::led::{LEDState, LedEvent},
    spoof::sensor::EmuBoardState,
};

/// This is a u8 because of -fshort-enums.
#[derive(PartialEq, Eq, PartialOrd, Ord, Clone, Copy, Debug)]
#[repr(u8)]
pub enum PieceType {
    WhitePawn,
    WhiteRook,
    WhiteKnight,
    WhiteBishop,
    WhiteQueen,
    WhiteKing,
    EmptySquare,
    BlackKing,
    BlackQueen,
    BlackBishop,
    BlackKnight,
    BlackRook,
    BlackPawn,
}

impl PieceType {
    pub fn color_change(&self) -> Self {
        (PieceType::BlackPawn as u8 - *self as u8)
            .try_into()
            .unwrap()
    }
}

impl TryFrom<u8> for PieceType {
    type Error = ();

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        if value > PieceType::BlackPawn as u8 {
            Err(())
        } else {
            Ok(unsafe { std::mem::transmute::<u8, PieceType>(value) })
        }
    }
}

#[derive(Clone, Copy)]
pub enum UserEvent {
    SetSquare(Square, PieceType),
    ButtonPress(ButtonNum),
    TimeUp,
    Quit,
}

#[derive(Clone, Copy)]
pub struct Square {
    pub row: usize,
    pub col: usize,
}

impl Display for Square {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}{}",
            (b'A' + self.col as u8) as char,
            (b'1' + self.row as u8) as char
        )
    }
}

#[derive(Clone, Copy)]
/// Events that should be sent to the emulator logic.
pub enum EmuEvent {
    Led(LedEvent),
    ResendSensor,
    User(UserEvent),
    UartFromPi(u32),
    UartToPi(u32),
}

pub enum UIEvent {
    LEDChange(Box<LEDState>),
    BoardChange(EmuBoardState),
    Quit,
}

/// Sent to the main thread from the emulator.
#[derive(Debug)]
pub enum MainEvent {
    SensorUpdate(EmuBoardState),
    ButtonPress(ButtonNum),
    UartMessage(u32),
    ClockTimeover,
    Quit,
}

pub enum UartEvent {
    Packet(u32),
    Quit,
}

/// For sending messages to the emulator.
pub static EMU_CHANNELS: Lazy<(Sender<EmuEvent>, Mutex<Receiver<EmuEvent>>)> = Lazy::new(|| {
    let (send, recv) = channel();
    (send, Mutex::new(recv))
});

/// For the emulator to send messages back to the UI.
pub static UI_CHANNELS: Lazy<(Sender<UIEvent>, Mutex<Receiver<UIEvent>>)> = Lazy::new(|| {
    let (send, recv) = channel();
    (send, Mutex::new(recv))
});

/// For sending messages to the main thread.
pub static MAIN_CHANNELS: Lazy<(Sender<MainEvent>, Mutex<Receiver<MainEvent>>)> = Lazy::new(|| {
    let (send, recv) = channel();
    (send, Mutex::new(recv))
});

/// For sending messages to the UART process.
pub static UART_CHANNELS: Lazy<(Sender<UartEvent>, Mutex<Receiver<UartEvent>>)> = Lazy::new(|| {
    let (send, recv) = channel();
    (send, Mutex::new(recv))
});

// Convenience functions for me.
pub fn send_emu(event: EmuEvent) {
    EMU_CHANNELS.0.send(event).expect("Emu channel closed?");
}
