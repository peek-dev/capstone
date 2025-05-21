/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * button-v2.c: provides button mapping structures for the button driver.
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
#include "private/button_private.h"
#if HARDWARE_REVISION == 2

const uint32_t prvButton_PinsA[PRVBUTTON_LENA] = {
    BUTTON_GPIO_PIN_PAUSE_PIN, BUTTON_GPIO_PIN_START_RESTART_PIN,
    BUTTON_GPIO_PIN_CLOCK_MODE_PIN, BUTTON_GPIO_PIN_BLACK_MOVE_PIN,
    BUTTON_GPIO_PIN_WHITE_MOVE_PIN};
const enum button_num prvButton_ButtonsA[PRVBUTTON_LENA] = {
    button_num_pause, button_num_start_restart, button_num_clock_mode,
    button_num_white_move, button_num_black_move};
const uint32_t prvButton_PinsB[PRVBUTTON_LENB] = {BUTTON_GPIO_PIN_UNDO_PIN,
                                 BUTTON_GPIO_PIN_HINT_PIN};
const enum button_num prvButton_ButtonsB[PRVBUTTON_LENB] = {button_num_undo, button_num_hint};

#endif