use std::sync::mpsc::{channel, Receiver, Sender};

use once_cell::sync::Lazy;
use parking_lot::Mutex;

use crate::event::EmuEvent;
