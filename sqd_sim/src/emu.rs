use std::sync::mpsc::{channel, Receiver, Sender};

use once_cell::sync::Lazy;
use parking_lot::Mutex;

use crate::{event::{send_main, EmuEvent, MainEvent, EMU_CHANNELS}, spoof::{led::emu_LED_do_event, sensor::starting_board}};

pub fn emulator_thread() {
	println!("Emulator thread started.");
	let recv = EMU_CHANNELS.1.lock();
	let mut board = starting_board();
	loop {
		let event = recv.recv().expect("Emu channel closed?");
		match event {
			EmuEvent::LED(ledevent) => emu_LED_do_event(ledevent),
			EmuEvent::User(user_event) => match user_event {
				crate::event::UserEvent::SetSquare(square, piece_type) => {
					board[square.row][square.col] = piece_type;
					send_main(MainEvent::SensorUpdate(board));
				},
				crate::event::UserEvent::ButtonPress(button_num) => {
					send_main(MainEvent::ButtonPress(button_num));
				},
				crate::event::UserEvent::TimeUp => {
					send_main(MainEvent::ClockTimeover);
				},
			},
		}
	}
}