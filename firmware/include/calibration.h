/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * calibration.h: A header to define a calibration interface that "tunes" 
 * the MSPM0G3507 to the analog signal characteristics of the 
 * C.H.E.S.S.B.O.A.R.D. squares.
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

#ifndef CAPSTONE_CALIBRATION_H
#define CAPSTONE_CALIBRATION_H
#include "config.h"
#include "game.h"

const uint16_t *GetBins(uint8_t row, uint8_t col);
PieceType xValueToPiece(uint16_t value, uint8_t row, uint8_t col);
#endif
