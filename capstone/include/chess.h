#ifndef CAPSTONE_CHESS_H
#define CAPSTONE_CHESS_H

#include "config.h"
#include "led.h"

typedef uint32_t NormalMove;
typedef uint32_t UndoMove;

#include "game.h"

extern const Color Color_InvalidMove;
extern const Color Color_Loser;
extern const Color Color_Winner;
extern const Color Color_Draw;
extern const Color Color_PieceAdjust;

int16_t sFindMoveIndex(BoardState *old, BoardState *new, NormalMove *moves,
                       uint16_t moves_len, BaseType_t whiteToMove);
BaseType_t xCheckValidMove(BoardState *old, BoardState *new, NormalMove move,
                           BaseType_t whiteToMove);
BaseType_t xCheckUndo(BoardState *old, BoardState *new, UndoMove move,
                      BaseType_t whiteToMove);
BaseType_t xFindSingleLifted(BoardState *old, BoardState *new,
                             uint8_t *found_row, uint8_t *found_col);
BaseType_t xIlluminateMove(NormalMove move, uint8_t do_src);
BaseType_t xIlluminateMovable(NormalMove *moves, uint16_t moves_len);
BaseType_t xIlluminatePieceMoves(NormalMove *moves, uint16_t moves_len,
                                 uint8_t row, uint8_t col);
BaseType_t xIlluminateUndo(UndoMove move, BaseType_t mv2ontop);
void vFlashDifferent(BoardState *old, BoardState *new);
#endif