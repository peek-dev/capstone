#include "sensor_mutex.h"

SemaphoreHandle_t sensor_mutex;

void vSensorMutex_Init(void) {
    sensor_mutex = xSemaphoreCreateMutex();
}