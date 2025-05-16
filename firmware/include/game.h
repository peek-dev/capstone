/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * button.h: definition of important game-related structures and functions.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */

#ifndef CAPSTONE_GAME_H
#define CAPSTONE_GAME_H

#include "config.h"
#include "portmacro.h"

typedef enum {
    WhitePawn,
    WhiteRook,
    WhiteKnight,
    WhiteBishop,
    WhiteQueen,
    WhiteKing,
    EmptySquare,
    BlackKing,
    BlackQueen,
    BlackBishop,
    BlackKnight,
    BlackRook,
    BlackPawn,
    PositioningError,
} PieceType; // 4bits

// Each uint32_t is a packed group of eight PieceTypes (4b each)
typedef struct {
    uint32_t rows[8];
} BoardState;

typedef struct {
    uint16_t columns[8];
} CalibrationRow;

typedef struct {
    CalibrationRow rows[8];
} BoardState_Calibration;

#include "chess.h"
typedef enum {
    game_turn_white = 0,
    game_turn_black = 1,
    // Only used for signaling the clock.
    game_turn_over,
} game_turn;
typedef enum {
    game_clock_off,
    game_clock_90_plus30,
    game_clock_25_plus10,
    game_clock_5_plus3
} chess_clock_mode;
typedef struct {
    enum {
        game_state_paused,
        game_state_running,
        // note: undo is only enabled when the clock is off.
        game_state_undo,
        game_state_notstarted,
        game_state_over,
    } state;
    game_turn turn;
    chess_clock_mode clock_mode;
    enum {
        game_hint_unknown,
        game_hint_known,
        game_hint_awaiting,
        game_hint_displaying
    } hint;
    NormalMove hint_move;
    BoardState last_move_state;
    BoardState last_measured_state;
    uint8_t check_row;
    uint8_t check_col;
    BaseType_t in_check;
    BaseType_t awaiting_undo;
    enum {
        game_winner_black = game_turn_white,
        game_winner_white = game_turn_black,
        game_winner_draw,
    } winner;
} GameState;

// OPT compact row and column into one field.
PieceType xGetSquare(BoardState *board, uint8_t row, uint8_t column);
void vSetSquare(BoardState *board, uint8_t row, uint8_t column, PieceType type);
BaseType_t xBoardEqual(BoardState *a, BoardState *b);
BaseType_t isWhite(PieceType p);
PieceType xChangeColor(PieceType p);
void vBoardSetDefault(BoardState *board);
#endif
