#include "config.h"
#include <task.h>

#include "led.h"
#include "projdefs.h"

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
            color.brightness = 1;

            xReturned = xLED_clear_board();
            while (xReturned != pdPASS) {
            }

            for (uint8_t k = 0; k < NUM_LEDS; k++) {
                xReturned = xLED_set_color(k, &color);
                while (xReturned != pdPASS) {
                }
                color.brightness += 4;
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