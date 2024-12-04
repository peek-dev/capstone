#include "config.h"
#include <task.h>

#include "clock.h"
#include "main.h"

uint16_t numbers[2] = {0};

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

BaseType_t xMain_button_press(enum button_num button) { return pdTRUE; }

BaseType_t xMain_uart_message(uint32_t move) { return pdTRUE; }

BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state) { return pdTRUE; }