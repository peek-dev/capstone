use std::sync::mpsc::{channel, Receiver, Sender};

use once_cell::sync::Lazy;
use parking_lot::Mutex;

use crate::{
    spoof::button::ButtonNum,
    spoof::led::{LEDEvent, LEDState},
    spoof::sensor::EmuBoardState,
};

/// This is a u8 because of -fshort-enums.
#[derive(PartialEq, Eq, PartialOrd, Ord, Clone, Copy)]
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

pub enum UserEvent {
    SetSquare(Square, PieceType),
    ButtonPress(Button),
    TimeUp,
}

pub struct Square {
    row: u8,
    col: u8,
}

#[repr(u8)]
pub enum Button {
    Restart = 1,
    Hint = 2,
    Undo = 3,
    Pause,
    BlackMove,
    WhiteMove,
    ClockMode,
}

/// Events that the main thread would send to the emulator logic.
pub enum EmuEvent {
    LED(LEDEvent),
    User(UserEvent),
}

pub enum UIEvent {
    LEDChange(LEDState),
}

/// Sent to the main thread from the emulator.
pub enum MainEvent {
    SensorUpdate(EmuBoardState),
    ButtonPress(ButtonNum),
    UARTMessage(u32),
    ClockTimeover,
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

pub fn send_emu(event: EmuEvent) {
    EMU_CHANNELS.0.send(event).expect("Emu channel closed?");
}

pub fn send_ui(event: UIEvent) {
    UI_CHANNELS.0.send(event).expect("UI channel closed?");
}

pub fn send_main(event: MainEvent) {
    MAIN_CHANNELS.0.send(event).expect("Main channel closed?");
}
