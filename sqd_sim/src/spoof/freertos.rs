use std::{sync::mpsc::RecvTimeoutError, time::Duration};

use crate::{
    event::{MainEvent, MAIN_CHANNELS},
    spoof::button::ButtonNum,
    spoof::sensor::BoardState,
    BaseType_t, UBaseType_t,
};

const PORT_MAX_DELAY: TickType_t = TickType_t::MAX;

#[repr(u8)]
pub enum MainThread_MsgType {
    SensorUpdate,
    ButtonPress,
    UARTMessage,
    ClockTimeover,
}

#[repr(C)]
pub union MainThread_MsgContents {
    state: BoardState,
    button: ButtonNum,
    chessmove: u32,
}

#[repr(C)]
pub struct MainThread_Message {
    msgtype: MainThread_MsgType,
    contents: MainThread_MsgContents,
}

impl From<MainEvent> for MainThread_Message {
    fn from(value: MainEvent) -> Self {
        let mut msg = MainThread_Message {
            msgtype: MainThread_MsgType::SensorUpdate,
            contents: MainThread_MsgContents { chessmove: 0 },
        };
        match value {
            MainEvent::SensorUpdate(bs) => {
                msg.msgtype = MainThread_MsgType::SensorUpdate;
                msg.contents.state = bs.into();
            }
            MainEvent::ButtonPress(button_num) => {
                msg.msgtype = MainThread_MsgType::ButtonPress;
                msg.contents.button = button_num;
            }
            MainEvent::UARTMessage(chessmove) => {
                msg.msgtype = MainThread_MsgType::UARTMessage;
                msg.contents.chessmove = chessmove;
            }
            MainEvent::ClockTimeover => {
                msg.msgtype = MainThread_MsgType::ClockTimeover;
            }
        };
        msg
    }
}

type QueueHandle_t = usize;
/// Ticks = ms. Our SYSTICK was running at 1kHz.
type TickType_t = u32;
/// Clears the main thread's input queue.
/// Returns a dummy pointer, which should NEVER be dereferenced.
/// But because of how the error handling works, it can't be null.
///
/// The blatant type-unsafety disgusts me, but I suppose it's what
/// is necessary when trying to spoof C functions.
#[no_mangle]
pub extern "C" fn xQueueGenericCreate(
    _uxQueueLength: UBaseType_t,
    _uxItemSize: UBaseType_t,
    _ucQueueType: u8,
) -> QueueHandle_t {
    let recv = MAIN_CHANNELS.1.lock();
    while recv.try_recv().is_ok() {}
    1
}

/// This is the only one that really matters. This needs
/// to take an event from the internal queue and turn it into
/// an event for the C program to process.
#[no_mangle]
pub extern "C" fn xQueueReceive(
    _xQueue: QueueHandle_t,
    pvBuffer: *mut MainThread_Message,
    xTicksToWait: TickType_t,
) -> BaseType_t {
    let recv = MAIN_CHANNELS.1.lock();
    let payload = if xTicksToWait == PORT_MAX_DELAY {
        recv.recv().expect("Main channel closed?")
    } else {
        match recv.recv_timeout(Duration::from_millis(xTicksToWait.into())) {
            Ok(p) => p,
            Err(RecvTimeoutError::Disconnected) => panic!("Main channel closed?"),
            Err(RecvTimeoutError::Timeout) => {
                return 0;
            }
        }
    };
    unsafe {
        *pvBuffer = payload.into();
    }
    1
}

/// Dummy function, needed for linking. Should never be called.
#[no_mangle]
pub extern "C" fn xQueueGenericSend(
    _xQueue: QueueHandle_t,
    _pvItemToQueue: usize,
    _xTicksToWait: TickType_t,
    _xCopyPosition: BaseType_t,
) -> BaseType_t {
    panic!("xQueueGenericSend should never be called! Examine stack trace.");
}

/// Dummy function, needed for linking. Should never be called.
#[no_mangle]
pub extern "C" fn xQueueGenericSendFromISR(
    _xQueue: QueueHandle_t,
    _pvItemToQueue: usize,
    _pxHigherPriorityTaskWoken: usize,
    _xCopyPosition: BaseType_t,
) -> BaseType_t {
    panic!("xQueueGenericSendFromISR should never be called! Examine stack trace.");
}

/// Dummy function, does not create the tasks.
/// The tasks are all being emulated already by the system.
#[no_mangle]
pub extern "C" fn xTaskCreate(
    _pxTaskCode: usize,
    _pcName: usize,
    _usStackDepth: u16,
    _pvParameters: usize,
    _uxPriority: UBaseType_t,
    _pxCreatedTask: usize,
) -> BaseType_t {
    1
}

/// To intercept the creation of the `sensor_mutex`.
/// Returns a dummy pointer, which should NEVER be dereferenced.
#[no_mangle]
pub extern "C" fn xQueueCreateMutex(_ucQueueType: u8) -> QueueHandle_t {
    1
}

/// I had forgotten how obsessively I would check the free memory remaining.
/// Anyway, here's something to catch those calls. Good thing I never did
/// anything with the result.
#[no_mangle]
pub extern "C" fn xPortGetFreeHeapSize() -> usize {
    0
}
