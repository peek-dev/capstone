use std::fmt::{Display, Write};

use colored::Colorize;

use crate::{
    event::{PieceType, UIEvent, UI_CHANNELS},
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

pub fn ui_thread() {
    let recv = UI_CHANNELS.1.lock();
    let mut leds = LEDState::default();
    let board = starting_board();
    loop {
        print!("{}", render_board(&board, &leds));
        let event = recv.recv().expect("UI channel closed?");
        match event {
            UIEvent::LEDChange(ledstate) => {
                leds = ledstate;
            }
            UIEvent::Quit => {
                println!("UI exiting");
                break;
            }
        }
    }
}
