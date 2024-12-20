/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * led_translation.h: definitions of functions to translate between logical and physical LEDs.
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

#ifndef CAPSTONE_LED_TRANSLATION_H
#define CAPSTONE_LED_TRANSLATION_H

#include "game.h"

typedef struct {
    uint8_t len;
    uint8_t data[2];
} ZeroToTwoInts;

ZeroToTwoInts LEDTrans_Ptype(PieceType p);
uint8_t LEDTrans_Square(uint8_t row, uint8_t col);
#endif
