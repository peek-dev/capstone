#include "config.h"
#include <task.h>

#include "clock.h"

void mainThread(void *arg0) {
    TaskHandle_t thread_clock;
    BaseType_t xReturned;

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {}

    // Make a thread to recieve messages.
    xReturned = xTaskCreate(vClock_Thread, "Clock", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_clock);
    // Loop and get stuck here if we fail.
    while (xReturned != pdPASS) {}

    xReturned = xClock_run_test(1);
    while (xReturned != pdPASS) {}
    vTaskDelete(NULL);
}

BaseType_t xMain_sensor_update(BoardState *state) { return pdTRUE; }

BaseType_t xMain_button_press(enum button_num button) { return pdTRUE; }

BaseType_t xMain_uart_message(uint32_t move) { return pdTRUE; }