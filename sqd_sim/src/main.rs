#![allow(non_camel_case_types)]

use std::{ptr::null_mut, thread::{self, spawn}};

use emu::emulator_thread;
use spoof::mainThread;

mod cli;
mod emu;
mod event;
pub mod spoof;

pub type BaseType_t = i32;
pub type UBaseType_t = u32;

fn main() {
	println!(
"Square Dance Simulator
======================
      🕺⬛⬜💃");
	let handle = spawn(emulator_thread);
	unsafe {
		spawn(|| mainThread(null_mut()));
	}
	handle.join().unwrap();
}
