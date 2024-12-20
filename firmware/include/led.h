/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * led.h: structures and functions related to signaling the LED thread.
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

#ifndef CAPSTONE_LED_H
#define CAPSTONE_LED_H

#include "config.h"

#define NUM_LEDS (82)

// Unlike the APA102c, the SK9822 brightness option changes the current (and
// hence the power draw :D) so it doesn't cause flickering.
typedef struct {
    // Warning: only 5 bits! Top 3 bits ignored.
    uint8_t brightness;
    // Full 8 bits.
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Color;

BaseType_t xLED_clear_board(void);
BaseType_t xLED_set_color(uint8_t num, const Color *pColor);
BaseType_t xLED_commit(void);
BaseType_t xLED_save(uint8_t save_num);
BaseType_t xLED_restore(uint8_t save_num);

BaseType_t xLED_Init(void);
void vLED_Thread(void *arg0);

#endif
