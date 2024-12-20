/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * TODO: add one-sentence description of what this does
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

#include "clock.h"
#include "main.h"

uint16_t numbers[2] = {0};

void mainThread(void *arg0) {
    TaskHandle_t thread_clock;
    BaseType_t xReturned;
    sensor_mutex = xSemaphoreCreateMutex();

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {}

    // Make a thread to recieve messages.
    xReturned = xTaskCreate(vClock_Thread, "Clock", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_clock);
    // Loop and get stuck here if we fail.
    while (xReturned != pdPASS) {}

    for (uint8_t i = 0; i < 3; i++) {
        switch (i) {
        case 1:
            numbers[0] = 65535;
            numbers[1] = 65535;
            break;
        case 2:
            numbers[0] = 1200;
            numbers[1] = 42;
            break;
        default:
            break;
        }
        xClock_set_numbers(numbers);
        xClock_set_state(clock_state_staticnumbers);
        vTaskDelay(1000);
    }
    xReturned = xClock_run_test(1);
    while (xReturned != pdPASS) {}
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
