/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * TODO: add about one sentence to describe what this source file does
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
#include "game.h"
#include "assert.h"
#include "config.h"
#include "projdefs.h"

PieceType xGetSquare(BoardState *board, uint8_t row, uint8_t column) {
    assert(row < 8 && column < 8);
    return (PieceType)(0xF & (board->rows[row] >> (4 * column)));
}
void vSetSquare(BoardState *board, uint8_t row, uint8_t column,
                PieceType type) {
    assert(row < 8 && column < 8);
    board->rows[row] &= ~(0xF << (4 * column));
    board->rows[row] |= type << (4 * column);
}

BaseType_t isWhite(PieceType p) { return (p <= WhiteKing) ? pdTRUE : pdFALSE; }

PieceType xChangeColor(PieceType p) {
    // This works only because the bins are symmetric around the middle (empty).
    return BlackPawn - p;
}

BaseType_t xBoardEqual(BoardState *a, BoardState *b) {
    for (uint8_t i = 0; i < 8; i++) {
        if (a->rows[i] != b->rows[i]) {
            return pdFALSE;
        }
    }
    return pdTRUE;
}

// The normal starting state for chess.
void vBoardSetDefault(BoardState *board) {
    board->rows[0] = 0x12354321ul;
    board->rows[1] = 0x00000000ul;
    for (uint8_t i = 2; i < 6; i++) {
        board->rows[i] = 0x66666666ul;
    }
    board->rows[6] = 0xccccccccul;
    board->rows[7] = 0xba9789abul;
}
