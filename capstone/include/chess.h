#ifndef CAPSTONE_CHESS_H
#define CAPSTONE_CHESS_H

#include "config.h"

typedef uint32_t NormalMove;

#include "game.h"

int16_t sFindMoveIndex(BoardState *old, BoardState *new, NormalMove *moves,
                       uint16_t moves_len);
BaseType_t xCheckValidMove(BoardState *old, BoardState *new, NormalMove move);
BaseType_t xFindSingleLifted(BoardState *old, BoardState *new, uint8_t *row,
                             uint8_t *column);
BaseType_t xIlluminateMove(NormalMove move, uint8_t do_src);

#endif