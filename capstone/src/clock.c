#include "config.h"

#include <string.h>
#include "portmacro.h"
#include "queue.h"
#include "assert.h"
#include "ti/driverlib/dl_gpio.h"
#include "ti/driverlib/dl_spi.h"

#define DELCLARE_PRIVATE_CLOCK_C
#include "clock.h"
#include "lcd.h"

// TODO maybe lower to 3?
#define QUEUE_SIZE 6

// [white, black]
static uint32_t times_ms[2];
static enum {
    clock_white_turn,
    clock_black_turn,
    clock_paused_white,
    clock_paused_black,
} state = clock_paused_white;

QueueHandle_t clockQueue;
TaskHandle_t xClockTaskId = NULL;

void vApplicationTickHook(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // If the tick rate doesn't evenly divide 1000, you'll be in for a world of
    // trouble. Please make the tick rate reasonable.
    if (state == clock_white_turn || state == clock_black_turn) {
        // Abuse enums being numbers to index.
        times_ms[state] -= (1000 / configTICK_RATE_HZ);
        // If we've just finished a second, wakeup the thread to signal the LCD.
        if (times_ms[state] % 1000 == 0) {
            Clock_Message msg = {.type = clockmsg_writehw,
                                 .seconds_per_test = 0};
            xQueueSendFromISR(clockQueue, &msg, &xHigherPriorityTaskWoken);
        }
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

BaseType_t xClock_Init(void) {
    vLCD_Init();
    times_ms[0] = 0;
    times_ms[1] = 0;
    state = clock_paused_white;
    clockQueue = xQueueCreate(QUEUE_SIZE, sizeof(Clock_Message));
    if (clockQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

void vClock_Thread(void *arg0) {
    assert(xClockTaskId == NULL);
    xClockTaskId = xTaskGetCurrentTaskHandle();
    Clock_Message message;
    while (1) {
        if (xQueueReceive(clockQueue, &message, portMAX_DELAY) == pdTRUE) {
            switch (message.type) {
            case clockmsg_set_time:
                memcpy(times_ms, message.times, 2 * sizeof(uint32_t));
                break;
            case clockmsg_start:
                if (state == clock_paused_white) {
                    state = clock_white_turn;
                } else if (state == clock_paused_black) {
                    state = clock_black_turn;
                }
                break;
            case clockmsg_stop:
                if (state == clock_white_turn) {
                    state = clock_paused_white;
                } else if (state == clock_black_turn) {
                    state = clock_paused_black;
                }
                break;
            case clockmsg_writehw: {
                uint32_t data[3];
                vLCD_RenderTime(times_ms, data);
                vLCD_WriteHardware(data);
                break;
            }
            case clockmsg_set_turn:
                if (state == clock_paused_black ||
                    state == clock_paused_white) {
                    state = message.set_turn_black ? clock_paused_black
                                                   : clock_paused_white;
                } else {
                    state = message.set_turn_black ? clock_black_turn
                                                   : clock_white_turn;
                }
                break;
            case clockmsg_test_segments:
                state = clock_white_turn;
                vLCD_RunTestSequence(message.seconds_per_test);
                break;
            }
        }
    }
}

BaseType_t xClock_run_test(uint8_t seconds_per_test) {
    Clock_Message m;
    m.type = clockmsg_test_segments;
    m.seconds_per_test = seconds_per_test;
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

BaseType_t xClock_set_turn(uint8_t is_black) {
    Clock_Message m;
    m.type = clockmsg_set_turn;
    m.set_turn_black = is_black;
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

BaseType_t xClock_stop_clock() {
    Clock_Message m;
    m.type = clockmsg_stop;
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

BaseType_t xClock_start_clock() {
    Clock_Message m;
    m.type = clockmsg_start;
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

BaseType_t xClock_set_times(uint32_t *times_ms) {
    Clock_Message m;
    m.type = clockmsg_set_time;
    memcpy(m.times, times_ms, sizeof(uint32_t) * 2);
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}