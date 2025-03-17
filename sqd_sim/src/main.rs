#![allow(non_camel_case_types)]

use std::{
    env,
    fs::File,
    io::{BufRead, BufReader},
    path::PathBuf,
    ptr::null_mut,
    thread::spawn,
};

use emu::emulator_thread;
use event::{send_emu, EmuEvent, UserEvent};
use parser::parse_line;
use spoof::{mainThread, uart::uart_thread};
use ui::ui_thread;

mod emu;
mod event;
mod parser;
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
    let mut prelude = Vec::new();
    if let Some(fname) = env::args().nth(1) {
        println!("Reading command file.");
        for line in BufReader::new(File::open(fname).unwrap()).lines() {
            prelude.push(parse_line(&line.unwrap()).unwrap());
        }
    }
    let wr = spawn(|| uart_thread(sf_wrapper));
    let emu = spawn(emulator_thread);
    let ui = spawn(|| ui_thread(prelude));
    let main = unsafe { spawn(|| mainThread(null_mut())) };

    ctrlc::set_handler(|| send_emu(EmuEvent::User(UserEvent::Quit)))
        .expect("Couldn't set ctrlc handler?");

    wr.join().unwrap();
    emu.join().unwrap();
    ui.join().unwrap();
    main.join().unwrap();
}
