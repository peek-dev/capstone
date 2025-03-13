#[repr(u8)]
#[derive(Clone, Copy)]
pub enum ButtonNum {
    StartRestart = 1,
    Hint = 2,
    Undo = 3,
    Pause,
    BlackMove,
    WhiteMove,
    ClockMode,
}
