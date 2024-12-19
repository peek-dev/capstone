#ifndef CAPSTONE_SENSOR_H
#define CAPSTONE_SENSOR_H

#include "config.h"

BaseType_t xSensor_Init(void);
void vSensor_Thread(void *arg0);
void vSensor_Thread_Calibration(void *arg0);

#endif