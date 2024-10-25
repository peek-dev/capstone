#ifndef CAPSTONE_CHESS_H
#define CAPSTONE_CHESS_H

typedef uint32_t NormalMove;

sFindMoveIndex(BoardState *old, BoardState *new, NormalMove *moves, uint16_t moves_len);
BaseType_t xCheckValidMove(BoardState *old, BoardState *new, NormalMove move);

#endif