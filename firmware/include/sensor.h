/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * sensor.h: interface for initializing and running the sensor thread.
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
#ifndef CAPSTONE_SENSOR_H
#define CAPSTONE_SENSOR_H

#include "config.h"

BaseType_t xSensor_Init(void);
void vSensor_Thread(void *arg0);
void vSensor_Thread_Calibration(void *arg0);

#endif
