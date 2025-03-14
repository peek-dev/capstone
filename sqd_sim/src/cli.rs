use crate::{event::UI_CHANNELS, spoof::{led::LEDState, sensor::starting_board}};


pub fn cli_thread() {
	let recv = UI_CHANNELS.1.lock();
	let mut leds = LEDState::default();
	let mut board = starting_board();
	loop {
		let event = recv.recv().expect("UI channel closed?");
		match event {
			crate::event::UIEvent::LEDChange(ledstate) => todo!(),
		}
	}
}
