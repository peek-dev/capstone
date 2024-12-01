#ifndef CAPSTONE_CHESS_H
#define CAPSTONE_CHESS_H

#include "config.h"

typedef uint32_t NormalMove;
typedef uint32_t UndoMove;

#include "game.h"

int16_t sFindMoveIndex(BoardState *old, BoardState *new, NormalMove *moves,
                       uint16_t moves_len, BaseType_t whiteToMove);
BaseType_t xCheckValidMove(BoardState *old, BoardState *new, NormalMove move, BaseType_t whiteToMove);
BaseType_t xFindSingleLifted(BoardState *old, BoardState *new,
                             uint8_t *found_row, uint8_t *found_col);
BaseType_t xIlluminateMove(NormalMove move, uint8_t do_src);
BaseType_t xIlluminateMovable(NormalMove *moves, uint16_t moves_len);
BaseType_t xIlluminatePieceMoves(NormalMove *moves, uint16_t moves_len, uint8_t row, uint8_t col);
#endif