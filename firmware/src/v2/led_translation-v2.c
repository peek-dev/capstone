/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * led_translation.c: functions to translate from logical chess squares
 *                    and piece types to physical LED indices.
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
#include "led_translation.h"
#if HARDWARE_REVISION == 2

#include "game.h"
#include "assert.h"

static const ZeroToTwoInts PTYPE_TRANS[] = {
    {.len = 1, .data = {16, 0}},
    {.len = 2, .data = {0, 7}},
    {.len = 2, .data = {1, 6}},
    {.len = 2, .data = {2, 5}},
    {.len = 1, .data = {4, 0}},
    {.len = 1, .data = {3, 0}},
    {.len = 0, .data = {0, 0}},
    {.len = 1, .data = {78, 0}},
    {.len = 1, .data = {77, 0}},
    {.len = 2, .data = {76, 79}},
    {.len = 2, .data = {75, 80}},
    {.len = 2, .data = {74, 81}},
    {.len = 1, .data = {57, 0}},
    {.len = 0, .data = {0, 0}},
};

ZeroToTwoInts LEDTrans_Ptype(PieceType p) {
    return PTYPE_TRANS[p];
}

uint8_t LEDTrans_Square(uint8_t row, uint8_t col) {
    assert(row < 8 && col < 8);
    // In the comments I will use one-based row numbers (like in chess), but in the
    // code I will use zero-based row numbers.
    // Skip the first row of pieces, a pawn if row>=2, and another if row>=7
    uint8_t index = 8 + (row >= 1 ? 1 : 0) + (row >= 6 ? 1 : 0);
    // Offset by the number of rows.
    index += 8 * row;
    // Every other row goes backward, starting with row 2 (1).
    index += (row % 2 != 0) ? (7 - col) : col;
    return index;
}

#endif