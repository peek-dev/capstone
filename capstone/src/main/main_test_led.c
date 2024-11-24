#include "config.h"
#include <task.h>

#include "led.h"
#include "led_translation.h"

void mainThread(void *arg0) {
    TaskHandle_t thread_led;
    BaseType_t xReturned;

    /* Call driver init functions */
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {
    }

    // Make a thread to recieve messages.
    xReturned = xTaskCreate(vLED_Thread, "LED", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_led);
    // Loop and get stuck here if we fail.
    while (xReturned != pdPASS) {
    }

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
            color.brightness = j % 32;
            Color color_copy = color;

            xReturned = xLED_clear_board();
            while (xReturned != pdPASS) {
            }

            for (uint8_t row = 0; row < 8; row++) {
                for (uint8_t col = 0; col < 8; col++) {
                    xReturned =
                        xLED_set_color(LEDTrans_Square(row, col), &color_copy);
                    while (xReturned != pdPASS) {
                    }
                }
                color.brightness = (color.brightness + 4) % 32;
                color_copy.red = color_copy.red <= 4 ? 0 : (color_copy.red - 4);
                color_copy.green =
                    color_copy.green <= 4 ? 0 : (color_copy.green - 4);
                color_copy.blue =
                    color_copy.blue >= 251 ? 255 : (color_copy.blue + 4);
            }
            xReturned = xLED_commit();
            while (xReturned != pdPASS) {
            }

            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
        i++;
    }
    vTaskDelete(NULL);
}

BaseType_t xMain_sensor_update(BoardState *state) { return pdTRUE; }

BaseType_t xMain_button_press(enum button_num button) { return pdTRUE; }

BaseType_t xMain_uart_message(uint32_t move) { return pdTRUE; }