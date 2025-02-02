/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * main_test_led.c: alternative event loop to cycle through a rainbow
 * of colors on the squares of the chessboard.
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

    Color color = {.brightness = 31, .red = 255, .blue = 0, .green = 0};
    int8_t masks[3][3] = {{-1, 1, 0}, {0, -1, 1}, {1, 0, -1}};
    uint8_t i = 0;
    while (1) {
        if (i == 3) {
            i = 0;
        }
        for (uint16_t j = 0; j < 255; j++) {
            color.red += masks[i][0];
            color.green += masks[i][1];
            color.blue += masks[i][2];
            color.brightness = j / 8;
            Color color_copy = color;

            xReturned = xLED_clear_board();
            while (xReturned != pdPASS) {}

            for (uint8_t row = 0; row < 8; row++) {
                for (uint8_t col = 0; col < 8; col++) {
                    color_copy.brightness = (color_copy.brightness + 4) % 32;
                    xReturned =
                        xLED_set_color(LEDTrans_Square(row, col), &color_copy);
                    while (xReturned != pdPASS) {}
                }
                color_copy.red = color_copy.red <= 4 ? 0 : (color_copy.red - 4);
                color_copy.green =
                    color_copy.green <= 4 ? 0 : (color_copy.green - 4);
                color_copy.blue =
                    color_copy.blue >= 251 ? 255 : (color_copy.blue + 4);
            }
            xReturned = xLED_commit();
            while (xReturned != pdPASS) {}

            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
        i++;
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
