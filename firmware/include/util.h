/** 
 * Copyright (C) 2024 John E. Berberian, Jr.  
 *
 * util.h: An interface definition for a helper function that takes the median 
 * of five measurements, used here to take the median of five sensor readings.
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <https://www.gnu.org/licenses/>
 */
#ifndef CAPSTONE_UTIL_H 
#define CAPSTONE_UTIL_H 
#include "game.h"

uint16_t MedianOfFive(uint16_t *arr);
#endif
