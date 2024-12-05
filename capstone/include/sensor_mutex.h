#ifndef CAPSTONE_SENSOR_MUTEX_H
#define CAPSTONE_SENSOR_MUTEX_H

#include "semphr.h"

#define LCD_USE_SENSOR_MUTEX 0
#define UART_USE_SENSOR_MUTEX 0
#define LED_USE_SENSOR_MUTEX 1

extern SemaphoreHandle_t sensor_mutex;

#endif