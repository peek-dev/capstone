/**
 * Copyright (C) <year>  <name of author>
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
#include "config.h"
#include "led_translation.h"
#include "game.h"
#include "assert.h"

ZeroToTwoInts LEDTrans_Ptype(PieceType p) {
    ZeroToTwoInts z;
    z.len = 2;
    z.data[0] = 0;
    z.data[1] = 7;
    switch (p) {
    case EmptySquare:
        z.len -= 1;
    case WhiteQueen:
        z.data[0] += 1;
    case WhiteKing:
        z.data[0] += 1;
        z.len -= 1;
    case WhiteBishop:
        // Shift one in.
        z.data[0] += 1;
        z.data[1] -= 1;
    case WhiteKnight:
        // Shift one in.
        z.data[0] += 1;
        z.data[1] -= 1;
    case WhiteRook:
        // Shift up one row and then one.
        z.data[0] += 1 * 8 + 1;
        // The other one is still down at BlackBishop.
        // Two to move to rook, nine rows, two pawns.
        z.data[1] += 2 + 9 * 8 + 2;
        z.len += 1;
    case WhitePawn:
        // Shift up by eight rows, and then one (one after)
        z.data[0] += 5 * 8 + 1;
    case BlackPawn:
        // Skip the rest of the piece row (3), skip two rows, and shift one.
        z.data[0] += 3 + 2 * 8 + 1;
    case BlackKing:
        // Shift one up.
        z.data[0] += 1;
    case BlackQueen:
        // Shift one up, and decrease the length.
        z.data[0] += 1;
        z.len -= 1;
    case BlackBishop:
        // Shift one in.
        z.data[0] += 1;
        z.data[1] -= 1;
    case BlackKnight:
        // Shift one in.
        z.data[0] += 1;
        z.data[1] -= 1;
    case BlackRook:
        // Initial state. Change nothing.
        break;
    }
    return z;
}

uint8_t LEDTrans_Square(uint8_t row, uint8_t col) {
    assert(row < 8 && col < 8);
    // Everything flipped since I wrote this function. Hacky fix.
    // I don't feel like rewriting this, and the compiler will optimize
    // it out anyway.
    row = 7 - row;
    col = 7 - col;
    // Skip the first row of pieces, a pawn if row>=3, and another if row>=8
    uint8_t index = 8 + (row >= 2 ? 1 : 0) + (row == 7 ? 1 : 0);
    // Offset by the number of rows.
    index += 8 * row;
    // Even rows go forward, odd rows go backward.
    index += (row % 2 == 0) ? col : (7 - col);
    return index;
}
