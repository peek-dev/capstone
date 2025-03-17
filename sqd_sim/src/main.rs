#![allow(non_camel_case_types)]

use std::{env, path::PathBuf, ptr::null_mut, thread::spawn};

use emu::emulator_thread;
use event::{send_emu, EmuEvent, UserEvent};
use spoof::{mainThread, uart::uart_thread};
use ui::ui_thread;

mod emu;
mod event;
pub mod spoof;
mod ui;

pub type BaseType_t = i32;
pub type UBaseType_t = u32;

fn main() {
    let sf_wrapper =
        PathBuf::from(env::var("SF_WRAPPER_PY").expect("Missing env var: SF_WRAPPER_PY"))
            .canonicalize()
            .expect("Project layout changed?");
    assert!(sf_wrapper.exists());
    println!(
        "Square Dance Simulator
======================
      🕺⬛⬜💃"
    );
    let wr = spawn(|| uart_thread(sf_wrapper));
    let emu = spawn(emulator_thread);
    let ui = spawn(ui_thread);
    let main = unsafe { spawn(|| mainThread(null_mut())) };

    ctrlc::set_handler(|| send_emu(EmuEvent::User(UserEvent::Quit)))
        .expect("Couldn't set ctrlc handler?");

    wr.join().unwrap();
    emu.join().unwrap();
    ui.join().unwrap();
    main.join().unwrap();
}
