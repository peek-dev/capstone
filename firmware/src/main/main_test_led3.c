/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * main_test_led3.c: tests LED strip continuity by illuminating every LED
 * in the order they are laid out on the physical strip.
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
#include "config.h"
#include <task.h>

#include "led.h"
#include "button.h"
#include "led_translation.h"
#include "main.h"

void mainThread(void *arg0) {
    TaskHandle_t thread_led;
    BaseType_t xReturned;
    sensor_mutex = xSemaphoreCreateMutex();

    /* Call driver init functions */
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {}

    // Make a thread to recieve messages.
    xReturned = xTaskCreate(vLED_Thread, "LED", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_led);
    // Loop and get stuck here if we fail.
    while (xReturned != pdPASS) {}

    Color color = {.brightness = 31, .red = 255, .blue = 255, .green = 255};
    uint8_t i = 0;
    while (1) {
        if (i == 0 || i == NUM_LEDS) {
            i = 0;
        }
        xReturned = xLED_clear_board();
        while (xReturned != pdPASS) {}
        xReturned = xLED_set_color(i, &color);
        while (xReturned != pdPASS) {}
        xReturned = xLED_commit();
        while (xReturned != pdPASS) {}
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

BaseType_t xMain_sensor_update(BoardState *state) { return pdTRUE; }

BaseType_t xMain_button_press_FromISR(enum button_num button,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    return pdTRUE;
}

BaseType_t xMain_uart_message(uint32_t move) { return pdTRUE; }

BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state) {
    return pdTRUE;
}

BaseType_t xMain_uart_message_FromISR(uint32_t move,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    return pdTRUE;
}
