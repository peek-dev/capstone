/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * button.h: an enum encoding and interface definition to interpret button 
 * presses on the LP-MSPM0G3507 as chess game events for this project.
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

#ifndef CAPSTONE_BUTTON_H
#define CAPSTONE_BUTTON_H

#include "config.h"
#include "uart_bidir_protocol.h"

enum button_num {
    button_num_start_restart = BUTTON_RESTART,
    button_num_hint = BUTTON_HINT,
    button_num_undo = BUTTON_UNDO,
    button_num_pause,
    button_num_black_move,
    button_num_white_move,
    button_num_clock_mode,
};

void vButton_Init(void);

#endif
