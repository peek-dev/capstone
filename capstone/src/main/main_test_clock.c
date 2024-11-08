#include "config.h"
#include <task.h>

#include "clock.h"

void mainThread(void *arg0) {
    TaskHandle_t thread_clock;
    BaseType_t xReturned;

    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {
    }

    xReturned = xPortGetFreeHeapSize();

    // Make a thread to recieve messages.
    xReturned = xTaskCreate(vClock_Thread, "Clock", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_clock);
    // Loop and get stuck here if we fail.
    while (xReturned != pdPASS) {
    }
    xReturned = xPortGetFreeHeapSize();

    xReturned = xClock_run_test(1);
    while (xReturned != pdPASS) {
    }
    vTaskDelete(NULL);
}