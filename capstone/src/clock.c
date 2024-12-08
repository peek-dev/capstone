#include "config.h"

#include <string.h>
#include "game.h"
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
static uint16_t numbers[2];
static uint32_t increment_ms = 0;
static game_turn turn;
static clock_state state = clock_state_off;

QueueHandle_t clockQueue;
TaskHandle_t xClockTaskId = NULL;

void vApplicationTickHook(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // If the tick rate doesn't evenly divide 1000, you'll be in for a world of
    // trouble. Please make the tick rate reasonable.
    if (state == clock_state_running) {
        // Abuse enums being numbers to index.
        times_ms[turn] -= (1000 / configTICK_RATE_HZ);
        // If we've just finished a second, wakeup the thread to signal the LCD.
        if (times_ms[state] % 1000 == 0) {
            Clock_Message msg = {.type = clockmsg_render_state};
            xQueueSendFromISR(clockQueue, &msg, &xHigherPriorityTaskWoken);
        }
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

BaseType_t xClock_Init(void) {
    vLCD_Init();
    times_ms[0] = 0;
    times_ms[1] = 0;
    state = clock_state_notstarted;
    turn = game_turn_white;
    clockQueue = xQueueCreate(QUEUE_SIZE, sizeof(Clock_Message));
    if (clockQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

static BaseType_t prvClock_render_state(void) {
    Clock_Message m;
    m.type = clockmsg_render_state;
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

void vClock_Thread(void *arg0) {
    assert(xClockTaskId == NULL);
    xClockTaskId = xTaskGetCurrentTaskHandle();
    Clock_Message message;
    clock_state old_state;
    while (1) {
        if (xQueueReceive(clockQueue, &message, portMAX_DELAY) == pdTRUE) {
            switch (message.type) {
            case clockmsg_set_time:
                memcpy(times_ms, message.times, 2 * sizeof(uint32_t));
                break;
            case clockmsg_set_state:
                old_state = state;
                state = message.state;
                // Rerender (fall through) if the state changed.
                if (old_state == state) {
                    break;
                }
            case clockmsg_render_state: {
                uint32_t data[3];
                vLCD_RenderState(data, state, turn, times_ms, numbers, increment_ms);
                vLCD_WriteHardware(data);
                break;
            }
            case clockmsg_set_numbers:
                memcpy(numbers, message.numbers, 2 * sizeof(uint16_t));
                // Request a rerender if we're displaying numbers.
                if (state == clock_state_staticnumbers || state == clock_state_undo) {
                    prvClock_render_state();
                }
                break;
            case clockmsg_set_turn:
                if (state == clock_state_running && turn != message.turn &&
                    increment_ms != 0) {
                    times_ms[message.turn] += increment_ms;
                    prvClock_render_state();
                } else if (state == clock_state_paused ||
                           state == clock_state_undo) {
                    // Request a render.
                    prvClock_render_state();
                }
                turn = message.turn;
                break;
            case clockmsg_test_segments:
                state = clock_state_running;
                turn = game_turn_white;
                vLCD_RunTestSequence(message.seconds_per_test);
                break;
            case clockmsg_set_increment:
                increment_ms = message.times[0];
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

BaseType_t xClock_set_turn(game_turn turn) {
    Clock_Message m;
    m.type = clockmsg_set_turn;
    m.turn = turn;
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

BaseType_t xClock_set_state(clock_state state) {
    Clock_Message m;
    m.type = clockmsg_set_state;
    m.state = state;
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

BaseType_t xClock_set_times(uint32_t *times_ms) {
    Clock_Message m;
    m.type = clockmsg_set_time;
    memcpy(m.times, times_ms, sizeof(uint32_t) * 2);
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

BaseType_t xClock_set_increment(uint32_t increment_ms) {
    Clock_Message m;
    m.type = clockmsg_set_increment;
    m.times[0] = increment_ms;
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}

BaseType_t xClock_set_numbers(uint16_t *numbers) {
    Clock_Message m;
    m.type = clockmsg_set_numbers;
    memcpy(m.numbers, numbers, sizeof(uint16_t) * 2);
    return xQueueSend(clockQueue, &m, portMAX_DELAY);
}