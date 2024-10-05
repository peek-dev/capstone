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
    Color color = {.brightness = 31, .red=255,.blue=0,.green=0};
    for (uint8_t i = 0; i < 8; i++) {
        xReturned = xLED_set_color(i, &color);
        while (xReturned != pdPASS) {}
    }
    xReturned = xLED_commit();
    while (xReturned != pdPASS) {}

    vTaskDelete(NULL);
}