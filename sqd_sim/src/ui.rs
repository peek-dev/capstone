use std::{
    fmt::{Display, Write},
    io::stdin,
    thread::{sleep, spawn},
    time::Duration,
};

use colored::Colorize;

use crate::{
    emu::wait_for_uart,
    event::{send_emu, EmuEvent, PieceType, Square, UIEvent, UserEvent, UI_CHANNELS},
    parser::{parse_line, Commands, Interactive},
    spoof::{
        led::{Color, LEDState},
        sensor::{starting_board, EmuBoardState},
    },
};

const STARTING_BOARD: EmuBoardState = starting_board();

impl Display for PieceType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let c = match self {
            PieceType::WhitePawn => '♙',
            PieceType::WhiteRook => '♖',
            PieceType::WhiteKnight => '♘',
            PieceType::WhiteBishop => '♗',
            PieceType::WhiteQueen => '♕',
            PieceType::WhiteKing => '♔',
            PieceType::EmptySquare => ' ',
            PieceType::BlackKing => '♚',
            PieceType::BlackQueen => '♛',
            PieceType::BlackBishop => '♝',
            PieceType::BlackKnight => '♞',
            PieceType::BlackRook => '♜',
            PieceType::BlackPawn => '♟',
        };
        f.write_char(c)
    }
}

fn color_string(s: String, c: Color, back_black: Option<bool>) -> String {
    let colored = s.bright_black();
    if c.is_on() {
        colored.on_truecolor(c.red_norm(), c.green_norm(), c.blue_norm())
    } else if let Some(black) = back_black {
        if black {
            colored.on_black()
        } else {
            colored.on_truecolor(200, 200, 200)
        }
    } else {
        colored
    }
    .to_string()
}

fn render_board(board: &EmuBoardState, leds: &LEDState) -> String {
    let mut builder = String::new();

    builder.push_str("┏━━━━━━━━━━━━━━┓\n");
    // Piece outlines in the back.
    builder.push_str("┃   ");
    for piece in STARTING_BOARD[STARTING_BOARD.len() - 1] {
        builder.push_str(&color_string(piece.to_string(), leds.get_pt(piece), None));
    }
    // Column letters
    builder.push_str("   ┃\n┃   ABCDEFGH   ┃\n┃");
    builder.push_str("  ┌────────┐  ┃\n┃");

    // Render the rows themselves.
    for i in (0..board.len()).rev() {
        // On the second row we should have a pawn outline on the left.
        if i == 1 {
            let pawn = PieceType::WhitePawn;
            builder.push_str(&color_string(pawn.to_string(), leds.get_pt(pawn), None));
        } else {
            builder.push(' ');
        }
        // We also put the row number.
        let chari = (b'1' + i as u8) as char;
        builder.push(chari);
        builder.push('│');

        // These are the squares themselves - the contents of the row.
        for j in 0..board[i].len() {
            builder.push_str(&color_string(
                board[i][j].to_string(),
                leds.board[i][j],
                Some((i + j) % 2 == 0),
            ));
        }

        builder.push('│');
        // The row number can go on the other side as well.
        builder.push(chari);
        // On the seventh row we also want a black pawn outline, on the right.
        if i == board.len() - 2 {
            let pawn = PieceType::BlackPawn;
            builder.push_str(&color_string(pawn.to_string(), leds.get_pt(pawn), None));
        } else {
            builder.push(' ');
        }
        builder.push_str("┃\n┃");
    }
    // Same as before: the letters for the columns and the back rank piece outlines.
    builder.push_str("  └────────┘  ┃\n┃");
    builder.push_str("   ABCDEFGH   ┃\n┃   ");
    for piece in STARTING_BOARD[0] {
        builder.push_str(&color_string(piece.to_string(), leds.get_pt(piece), None));
    }
    builder.push_str("   ┃\n");
    builder.push_str("┗━━━━━━━━━━━━━━┛\n");
    builder
}

/// Handles user input and output.
pub fn ui_thread(prelude: Vec<Interactive>) {
    // secretly two threads. shhh
    spawn(|| input_thread(prelude));
    let recv = UI_CHANNELS.1.lock();
    let mut leds = LEDState::default();
    let mut board = starting_board();
    loop {
        print!("{}", render_board(&board, &leds));
        let event = recv.recv().expect("UI channel closed?");
        match event {
            UIEvent::LEDChange(ledstate) => {
                leds = *ledstate;
            }
            UIEvent::Quit => {
                println!("UI exiting");
                break;
            }
            UIEvent::BoardChange(b) => board = b,
        }
    }
}

fn emu_set_square(square: Square, piece: PieceType) {
    send_emu(EmuEvent::User(UserEvent::SetSquare(square, piece)));
}

fn input_thread(prelude: Vec<Interactive>) {
    let mut hand: PieceType = PieceType::EmptySquare;
    let mut board = starting_board();

    // Returns a boolean value indicating whether to continue.
    let mut process_line = |line: Interactive| {
        let sync_square = |sq: Square, b: &EmuBoardState| emu_set_square(sq, b[sq.row][sq.col]);
        match line.command {
            Commands::Button { button } => send_emu(EmuEvent::User(UserEvent::ButtonPress(button))),
            Commands::Move { src, dest } => {
                board[dest.row][dest.col] = board[src.row][src.col];
                board[src.row][src.col] = PieceType::EmptySquare;
                sync_square(src, &board);
                sync_square(dest, &board);
            }
            Commands::Lift { target } => {
                hand = board[target.row][target.col];
                board[target.row][target.col] = PieceType::EmptySquare;
                sync_square(target, &board);
            }
            Commands::Drop { target } => {
                board[target.row][target.col] = hand;
                hand = PieceType::EmptySquare;
                sync_square(target, &board);
            }
            Commands::Place { target, ptype } => {
                board[target.row][target.col] = ptype;
                sync_square(target, &board);
            }
            Commands::Time => send_emu(EmuEvent::User(UserEvent::TimeUp)),
            Commands::Quit => {
                send_emu(EmuEvent::User(UserEvent::Quit));
                return false;
            }
            Commands::Delay { time_ms } => sleep(Duration::from_millis(time_ms)),
            Commands::WaitForUart { time_ms } => {
                wait_for_uart();
                sleep(Duration::from_millis(time_ms.unwrap_or_default()));
            }
        }
        true
    };

    for line in prelude {
        if !process_line(line) {
            // We've been given the command to quit. Return.
            return;
        }
    }

    // If we haven't quit by now, loop over stdin lines.
    for line in stdin().lines() {
        match parse_line(&line.expect("Unable to read stdin?")) {
            Ok(parsed) => {
                if !process_line(parsed) {
                    return;
                }
            }
            Err(e) => println!("Parsing error: {}", e),
        }
    }
}
