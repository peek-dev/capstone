#![allow(non_camel_case_types)]

use std::{
    ptr::null_mut,
    thread::{self, spawn},
};

use emu::emulator_thread;
use spoof::mainThread;
use ui::ui_thread;

mod emu;
mod event;
pub mod spoof;
mod ui;

pub type BaseType_t = i32;
pub type UBaseType_t = u32;

fn main() {
    println!(
        "Square Dance Simulator
======================
      🕺⬛⬜💃"
    );
    let handle = spawn(emulator_thread);
    spawn(ui_thread);
    unsafe {
        spawn(|| mainThread(null_mut()));
    }
    handle.join().unwrap();
}
