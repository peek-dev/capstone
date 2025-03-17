use std::{env, fmt::Display, fs::OpenOptions, io::Write, path::PathBuf};

use chrono::Local;
use lazy_static::lazy_static;
use parking_lot::Mutex;

use crate::{
    event::{
        EmuEvent, MainEvent, PieceType, UIEvent, UartEvent, UserEvent, EMU_CHANNELS, MAIN_CHANNELS,
        UART_CHANNELS, UI_CHANNELS,
    },
    spoof::{
        led::{LEDState, LedEvent, N_LED_SAVES},
        led_translation::lednum_to_string,
        sensor::starting_board,
    },
};

fn send_ui(event: UIEvent) {
    UI_CHANNELS.0.send(event).expect("UI channel closed?");
}

fn send_main(event: MainEvent) {
    MAIN_CHANNELS.0.send(event).expect("Main channel closed?");
}

fn send_uart(event: UartEvent) {
    UART_CHANNELS.0.send(event).expect("UART channel closed?");
}

/// The emulator thread sits at the center of everything. Every event goes through here.
/// It tracks and updates the state of the virtualized hardware. It also acts as the central
/// exchange point for all messages (events), which should simplify logging somewhat.
pub fn emulator_thread() {
    println!("Emulator thread started.");
    let recv = EMU_CHANNELS.1.lock();
    let mut board = starting_board();
    let logpath: PathBuf =
        PathBuf::from(env::var("SQD_SIM_LOGFILE").unwrap_or("sqd_sim.log".to_string()));
    let mut logfile = OpenOptions::new()
        .append(true)
        .create(true)
        .open(logpath)
        .expect("Emu: Failed to open logfile");
    loop {
        let event = recv.recv().expect("Emu channel closed?");
        writeln!(logfile, "{} Event: {}", Local::now().to_rfc2822(), event)
            .expect("Emu: unable to write to logfile");
        match event {
            EmuEvent::Led(ledevent) => process_led_event(ledevent),
            EmuEvent::ResendSensor => send_main(MainEvent::SensorUpdate(board)),
            EmuEvent::User(user_event) => match user_event {
                UserEvent::SetSquare(square, piece_type) => {
                    board[square.row][square.col] = piece_type;
                    send_main(MainEvent::SensorUpdate(board));
                    send_ui(UIEvent::BoardChange(board));
                }
                UserEvent::ButtonPress(button_num) => {
                    send_main(MainEvent::ButtonPress(button_num));
                }
                UserEvent::TimeUp => {
                    send_main(MainEvent::ClockTimeover);
                }
                UserEvent::Quit => {
                    send_ui(UIEvent::Quit);
                    send_main(MainEvent::Quit);
                    send_uart(UartEvent::Quit);
                    println!("Emu exiting");
                    break;
                }
            },
            EmuEvent::UartFromPi(word) => send_main(MainEvent::UartMessage(word)),
            EmuEvent::UartToPi(word) => send_uart(UartEvent::Packet(word)),
        }
    }
}

// These should only be used by the emulator thread. Mutex is there to make the compiler
// happy, not to suggest that this should be accessed from outside threads.
lazy_static! {
    static ref LED_STATE: Mutex<LEDState> = Default::default();
    static ref SAVED_STATE: Mutex<[LEDState; N_LED_SAVES]> = Default::default();
}

/// This function is only intended to be called by the core emulator thread.
fn process_led_event(event: LedEvent) {
    match event {
        LedEvent::ClearBoard => {
            *(LED_STATE.lock()) = LEDState::default();
        }
        LedEvent::SetColor(num, color) => {
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
        LedEvent::Commit => {
            send_ui(UIEvent::LEDChange(Box::new(*LED_STATE.lock())));
        }
        LedEvent::Save(save_num) => {
            assert!(save_num < N_LED_SAVES as u8);
            SAVED_STATE.lock()[save_num as usize] = *LED_STATE.lock();
        }
        LedEvent::Restore(save_num) => {
            assert!(save_num < N_LED_SAVES as u8);
            *LED_STATE.lock() = SAVED_STATE.lock()[save_num as usize];
        }
    }
}

impl Display for EmuEvent {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            EmuEvent::Led(ledevent) => match ledevent {
                LedEvent::SetColor(n, c) => {
                    write!(f, "LED: set {} to {:?}", lednum_to_string(*n), c)
                }
                _ => write!(f, "LED: {:?}", ledevent),
            },
            EmuEvent::ResendSensor => write!(f, "Sensor: sending current state"),
            EmuEvent::User(user_event) => match user_event {
                UserEvent::SetSquare(square, piece_type) => {
                    write!(f, "User: set {} to {:?}", square, piece_type)
                }
                UserEvent::ButtonPress(button_num) => {
                    write!(f, "User: press {:?}", button_num)
                }
                UserEvent::TimeUp => write!(f, "User: time up"),
                UserEvent::Quit => write!(f, "User: quitting"),
            },
            EmuEvent::UartFromPi(word) => write!(f, "UART:     {:#010x} M←R", word),
            EmuEvent::UartToPi(word) => write!(f, "UART M→R: {:#010x}", word),
        }
    }
}
