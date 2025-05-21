/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * calibration-v2.c: provides standardized bins to the sensor driver for the 
 *                   resistive sensing mechanism.
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
#include "calibration.h"
#if HARDWARE_REVISION == 2

#define NBINS 15

static const uint16_t bins[NBINS] = {0, 146, 438, 731, 1023, 1316, 1608, 1901, 2193, 2486, 2778, 3071, 3363, 3656, 3948};
static const PieceType types[NBINS] = {PositioningError, WhitePawn, WhiteRook, WhiteKnight, WhiteBishop, WhiteQueen, WhiteKing, BlackPawn, BlackRook, BlackKnight, BlackBishop, BlackQueen, BlackKing, PositioningError, EmptySquare};

PieceType xValueToPiece(uint16_t value, uint8_t row, uint8_t col) {
    (void)row;
    (void)col;
    uint8_t i;
    for (i = 1; i < NBINS; i++) {
        if (bins[i] > value) {
            break;
        }
    }

    return types[i - 1];
}

#endif