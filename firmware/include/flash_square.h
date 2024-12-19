/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * TODO: describe in about one sentence what this header does.
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

#ifndef CAPSTONE_FLASH_SQUARE_H
#define CAPSTONE_FLASH_SQUARE_H

#include "config.h"
#include "led.h"

BaseType_t xFlashSquare_Init();

BaseType_t xFlashSquare_Enable(uint8_t led_num, uint16_t half_period_ticks,
                               Color color);
BaseType_t xFlashSquare_Disable(uint8_t led_num);
BaseType_t xFlashSquare_DisableAll();

void vFlashSquare_Thread(void *arg0);
#endif
