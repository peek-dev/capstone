/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * chess.h: a header and interface definition to support most chess game 
 * functionality in the C.H.E.S.S.B.O.A.R.D. firmware for this project.
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
extern const Color Color_Check;
extern const Color Color_PieceAdjust;

int16_t sFindMoveIndex(BoardState *old, BoardState *new, NormalMove *moves,
                       uint16_t moves_len, BaseType_t whiteToMove, BaseType_t *is_partial);
BaseType_t xCheckValidMove(BoardState *old, BoardState *new, NormalMove move,
                           BaseType_t whiteToMove, BaseType_t *is_partial);
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
BaseType_t xIlluminatePotentiallyOffCenter(BoardState *old, BoardState *new,
                                           BaseType_t *changed);
void vInvalidDifferent(BoardState *old, BoardState *new);
BaseType_t xIlluminatePartial(NormalMove move, BaseType_t whiteMove);
#endif
