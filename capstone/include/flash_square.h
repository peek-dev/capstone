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