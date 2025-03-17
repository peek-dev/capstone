use std::{
    env,
    ffi::c_void,
    io::{ErrorKind, Read, Write},
    path::PathBuf,
    process::{ChildStdout, Command, Stdio},
    thread::{sleep, spawn},
    time::Duration,
};

use crate::{
    event::{send_emu, EmuEvent, PieceType, UartEvent, UART_CHANNELS},
    BaseType_t,
};

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
const SIM_EXIT: u32 = 0x000000FF;

pub fn uart_thread(sf_wrapper: PathBuf) {
    // This is secretly two threads, one for each direction. Shhh.
    // Spawn the RPi process
    let mut pi_process =
        Command::new(env::var("SF_WRAPPER_PYTHON3").unwrap_or("python3".to_string()))
            .arg(sf_wrapper)
            .env("SQD_SIM", "Wake up, Neo.")
            .env("PYTHONUNBUFFERED", "1")
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .spawn()
            .expect("Unable to spawn sf_wrapper.py!");
    let stdout = pi_process
        .stdout
        .take()
        .expect("Failed to open sf_wrapper stdout");
    spawn(move || uart_listener(stdout));
    let recv = UART_CHANNELS.1.lock();
    let mut pipe = pi_process
        .stdin
        .take()
        .expect("Falled to open sf_wrapper stdin");
    loop {
        let event = recv.recv().expect("UART channels closed?");
        match event {
            UartEvent::Packet(word) => {
                let packet = word.to_le_bytes();
                pipe.write_all(&packet).expect("sf_wrapper crashed?");
            }
            UartEvent::Quit => {
                if let Err(_) = pipe.write_all(&SIM_EXIT.to_le_bytes()) {
                    pi_process.kill().expect("Unable to kill sf_wraper");
                }
                println!("Uart main exiting");
                break;
            }
        }
    }
}

fn uart_listener(mut pipe: ChildStdout) {
    loop {
        // No chance of framing errors here, we can just read directly.
        let mut packet = [0; 4];
        match pipe.read_exact(&mut packet) {
            Ok(_) => {
                let word = u32::from_le_bytes(packet);
                send_emu(EmuEvent::UartFromPi(word));
                if word & 1 == 1 {
                    send_emu(EmuEvent::ResendSensor);
                }
            }
            Err(n) => match n.kind() {
                ErrorKind::UnexpectedEof => {
                    // If the process exited, that is because we should quit.
                    println!("Uart child exiting");
                    break;
                }
                _ => Err(n).unwrap(),
            },
        }
    }
}

// Soon™ this will hook together with the python stockfish wrapper from the RPi software.
// But for now all it does is print out whatever gets sent. Sorry :(
#[no_mangle]
pub extern "C" fn xUART_Init() -> BaseType_t {
    1
}

#[no_mangle]
pub extern "C" fn xUART_to_wire(word: u32) -> BaseType_t {
    send_emu(EmuEvent::UartToPi(word));
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
pub extern "C" fn xPtypeFromWire(pt_in: PTYPE, whiteToMove: BaseType_t) -> PieceType {
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
