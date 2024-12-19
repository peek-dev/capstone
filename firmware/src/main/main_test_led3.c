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