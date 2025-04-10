/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 * config.h: a general configuration header for the MSPM0G3507-based firmware 
 * in this capstone project.
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

#ifndef CAPSTONE_CONFIG_H
#define CAPSTONE_CONFIG_H

// For the moment, allow assertions despite the code size and memory overhead.
#undef NDEBUG
#define MAKEVISIBLE volatile
#include <inttypes.h>
#include "ti_msp_dl_config.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portmacro.h"

#define IS_MSP
#define CAPSTONE_REVISION 2

#endif
