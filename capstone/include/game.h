#ifndef CAPSTONE_GAME_H
#define CAPSTONE_GAME_H

#include "config.h"

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

// OPT compact row and column into one field.
PieceType xGetSquare(BoardState *board, uint8_t row, uint8_t column);
void vSetSquare(BoardState *board, uint8_t row, uint8_t column, PieceType type);
BaseType_t isWhite(PieceType p);
PieceType xChangeColor(PieceType p);

#endif