#ifndef CAPSTONE_CHESS_H
#define CAPSTONE_CHESS_H

#include "config.h"
#include "game.h"

typedef uint32_t NormalMove;

int16_t sFindMoveIndex(BoardState *old, BoardState *new, NormalMove *moves,
                       uint8_t moves_len);
BaseType_t xCheckValidMove(BoardState *old, BoardState *new, NormalMove move);
BaseType_t xFindSingleLifted(BoardState *old, BoardState *new, uint8_t *row, uint8_t *column);

#endif