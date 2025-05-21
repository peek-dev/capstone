/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * sensor_mutex.h: definitions and macros to determine which portions
 * of the firmware may operate concurrently with sensor sampling.
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

#ifndef CAPSTONE_SENSOR_MUTEX_H
#define CAPSTONE_SENSOR_MUTEX_H

#include "config.h"
#include "semphr.h"

#define LCD_USE_SENSOR_MUTEX  0
#define UART_USE_SENSOR_MUTEX 0
#define LED_USE_SENSOR_MUTEX  1

extern SemaphoreHandle_t sensor_mutex;

void vSensorMutex_Init(void);

#endif
