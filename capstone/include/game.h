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
} PieceType; // 4bits

// Each uint32_t is a packed group of eight PieceTypes (4b each)
typedef struct {
    uint32_t rows[8];
} BoardState;

#include "chess.h"
typedef enum {
    game_turn_black = 1,
    game_turn_white = 0,
} game_turn;
typedef enum {
    game_clock_off,
    game_clock_90
    // TODO more
} chess_clock_mode;
typedef struct {
    enum {
        game_state_paused,
        game_state_running,
        // note: undo is only enabled when the clock is off.
        game_state_undo,
        game_state_notstarted,
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
} GameState;

// OPT compact row and column into one field.
PieceType xGetSquare(BoardState *board, uint8_t row, uint8_t column);
void vSetSquare(BoardState *board, uint8_t row, uint8_t column, PieceType type);
BaseType_t xBoardEqual(BoardState *a, BoardState *b);
BaseType_t isWhite(PieceType p);
PieceType xChangeColor(PieceType p);

#endif