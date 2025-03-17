use clap::{Parser, Subcommand};

use crate::{
    event::{PieceType, Square},
    spoof::button::ButtonNum,
};

#[derive(Parser)]
#[command(author, version, about, long_about = None, infer_subcommands = true)]
#[command(propagate_version = true)]
pub struct Interactive {
    #[command(subcommand)]
    pub command: Commands,
}

#[derive(Subcommand)]
pub enum Commands {
    /// Presses a button. Some buttons may not have implemented effects (e.g. clock).
    Button {
        #[arg(value_parser = parse_button)]
        button: ButtonNum,
    },
    /// Move the contents of a square to another square.
    /// This is similar to the combination of Lift and Drop, but without
    /// affecting the contents of your hand.
    Move {
        #[arg(value_parser = parse_square)]
        src: Square,
        #[arg(value_parser = parse_square)]
        dest: Square,
    },
    /// Lift a square's contents and hold it in your hand.
    Lift {
        #[arg(value_parser = parse_square)]
        target: Square,
    },
    /// Place the contents of your hand onto a square.
    /// This leaves EmptyPiece in your hand.
    Drop {
        #[arg(value_parser = parse_square)]
        target: Square,
    },
    /// Place a piece onto the board from some supply of pieces.
    Place {
        #[arg(value_parser = parse_square)]
        target: Square,
        #[arg(value_parser = parse_ptype)]
        ptype: PieceType,
    },
    /// Send the board a signal that the current player's time has run out.
    Time,
    /// Quit the emulator.
    Quit,
}

fn parse_square(arg: &str) -> Result<Square, String> {
    let s = arg.trim().to_ascii_lowercase();
    if s.len() != 2 {
        Err("Invalid square: must have length of 2.".to_string())
    } else {
        let col = s.chars().next().unwrap();
        let row = s.chars().nth(1).unwrap();
        if !('a'..='h').contains(&col) || !('1'..='8').contains(&row) {
            Err("Invalid square: must be a1-h8.".to_string())
        } else {
            // Yay should parse nicely!
            Ok(Square {
                row: row as usize - '1' as usize,
                col: col as usize - 'a' as usize,
            })
        }
    }
}

fn parse_button(arg: &str) -> Result<ButtonNum, String> {
    match arg.trim().to_ascii_lowercase().as_str() {
        "start" | "restart" => Ok(ButtonNum::StartRestart),
        "hint" => Ok(ButtonNum::Hint),
        "undo" => Ok(ButtonNum::Undo),
        "pause" | "unpause" => Ok(ButtonNum::Pause),
        "blackmove" | "black" => Ok(ButtonNum::BlackMove),
        "whitemove" | "white" => Ok(ButtonNum::WhiteMove),
        "clockmode" | "clock" => Ok(ButtonNum::ClockMode),
        _ => Err("Invalid button".to_string()),
    }
}

fn parse_ptype(arg: &str) -> Result<PieceType, String> {
    let trimmed = arg.trim();
    // For a single character,
    if trimmed.len() == 1 {
        let firstc = trimmed.chars().next().unwrap();
        let pt = algebraic_to_black_ptype(firstc)?;
        if firstc.is_uppercase() {
            Ok(pt.color_change())
        } else {
            Ok(pt)
        }
    } else {
        Err("Unsupported piece type notation: use PpRrNnBbQqKk, Ee".to_string())
    }
}

fn algebraic_to_black_ptype(c: char) -> Result<PieceType, String> {
    match c.to_ascii_lowercase() {
        'k' => Ok(PieceType::BlackKing),
        'n' => Ok(PieceType::BlackKnight),
        'b' => Ok(PieceType::BlackBishop),
        'r' => Ok(PieceType::BlackBishop),
        'q' => Ok(PieceType::BlackQueen),
        'p' => Ok(PieceType::BlackPawn),
        'e' => Ok(PieceType::EmptySquare),
        _ => Err("Invalid piece type.".to_string()),
    }
}

pub fn parse_line(line: &str) -> Result<Interactive, String> {
    let mut base = "sqdsim_prompt ".to_string();
    base.push_str(line);
    // Split the tokens according to standard shell rules
    let words = match shlex::split(&base) {
        None => {
            return Err("Failed to parse line.".to_string());
        }
        Some(v) => v,
    };
    Interactive::try_parse_from(words).map_err(|e| e.to_string())
}
