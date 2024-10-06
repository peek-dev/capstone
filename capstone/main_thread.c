#include <FreeRTOS.h>
#include <task.h>

#include "led.h"

void mainThread(void *arg0)
{
    TaskHandle_t thread_led;
    BaseType_t xReturned;

    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {}

    xReturned = xPortGetFreeHeapSize();

    // Make a thread to recieve messages.
    xReturned = xTaskCreate(vLED_Thread, "LED", configMINIMAL_STACK_SIZE, NULL, 2, &thread_led);
    // Loop and get stuck here if we fail.
    while (xReturned != pdPASS) {}
    xReturned = xPortGetFreeHeapSize();

    xReturned = xLED_clear_board();
    while (xReturned != pdPASS) {}
    Color color = {.brightness = 15, .red=255,.blue=0,.green=0};
    for (uint8_t i = 0; i < 12; i++) {
        if (i == 4) {
            color.red = 0;
            color.green = 255;
            color.brightness = 15;
        } else if (i == 8) {
            color.green = 0;
            color.blue = 255;
            color.brightness = 15;
        }
        xReturned = xLED_set_color(i, &color);
        while (xReturned != pdPASS) {}
        color.brightness = (color.brightness + 1)/2 - 1;
    }
    xReturned = xLED_commit();

    color.brightness = 15;
    while (xReturned != pdPASS) {}
    for (uint8_t i = 0; i < 12; i++) {
        if (i == 4) {
            color.blue = 0;
            color.red = 255;
            color.brightness = 15;
        } else if (i == 8) {
            color.red = 0;
            color.green = 255;
            color.brightness = 15;
        }
        xReturned = xLED_set_color(i, &color);
        while (xReturned != pdPASS) {}
        color.brightness = (color.brightness + 1)/2 - 1;
    }
    xReturned = xLED_commit();
    while (xReturned != pdPASS) {}

    color.brightness = 15;
    while (xReturned != pdPASS) {}
    for (uint8_t i = 0; i < 12; i++) {
        if (i == 4) {
            color.green = 0;
            color.blue = 255;
            color.brightness = 15;
        } else if (i == 8) {
            color.blue = 0;
            color.red = 255;
            color.brightness = 15;
        }
        xReturned = xLED_set_color(i, &color);
        while (xReturned != pdPASS) {}
        color.brightness = (color.brightness + 1)/2 - 1;
    }
    xReturned = xLED_commit();
    while (xReturned != pdPASS) {}

    vTaskDelete(NULL);
}