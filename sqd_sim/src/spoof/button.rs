#[repr(u8)]
#[derive(Clone, Copy, Debug)]
pub enum ButtonNum {
    StartRestart = 1,
    Hint = 2,
    Undo = 3,
    Pause,
    BlackMove,
    WhiteMove,
    ClockMode,
}

#[no_mangle]
pub extern "C" fn vButton_Init() {}
