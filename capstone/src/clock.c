#include "config.h"

#include <string.h>
#include "portmacro.h"
#include "queue.h"
#include "assert.h"
#include "ti/driverlib/dl_gpio.h"
#include "ti/driverlib/dl_spi.h"

#include "clock.h"

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
static QueueHandle_t clockQueue;

enum Clock_MsgType {
    clockmsg_set_time,
    clockmsg_writehw,
    clockmsg_stop,
    clockmsg_start,
    clockmsg_set_turn,
    clockmsg_test_segments,
};

typedef struct {
    // TODO: check that this is only using 8 bits, not more.
    enum Clock_MsgType type;
    union {
        uint32_t times[2];
        uint8_t seconds_per_test;
        uint8_t set_turn_black;
    };
} Clock_Message;
static TaskHandle_t xClockTaskId = NULL;

// [left, right], each element [Ones, tens]
static const uint8_t DIGITS[6][2] = {
    {DIGIT_1_2_OFFSET, DIGIT_1_1_OFFSET}, {DIGIT_1_4_OFFSET, DIGIT_1_3_OFFSET},
    {DIGIT_1_6_OFFSET, DIGIT_1_5_OFFSET}, {DIGIT_2_2_OFFSET, DIGIT_2_1_OFFSET},
    {DIGIT_2_4_OFFSET, DIGIT_2_3_OFFSET}, {DIGIT_2_6_OFFSET, DIGIT_2_5_OFFSET}};
static const uint8_t SEGMENTS[] = {SEVENSEG_A, SEVENSEG_B, SEVENSEG_C,
                                   SEVENSEG_D, SEVENSEG_E, SEVENSEG_F,
                                   SEVENSEG_G};
static const uint8_t NUMBERS[] = {
    SEVENSEG_0, SEVENSEG_1, SEVENSEG_2, SEVENSEG_3, SEVENSEG_4,
    SEVENSEG_5, SEVENSEG_6, SEVENSEG_7, SEVENSEG_8, SEVENSEG_9};

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

static void prvRenderTime(uint32_t *times, uint32_t *data);

static void prvTransmitHalfFrame(uint16_t half) {
    while (DL_SPI_isTXFIFOFull(CLOCK_SPI_INST)) {
        taskYIELD();
    }
    DL_SPI_transmitData16(CLOCK_SPI_INST, half);
}

static void prvTransmitFrame(uint32_t frame) {
    // frame = (frame >> 16) | (frame << 16);
    prvTransmitHalfFrame((frame >> 16) & 0xFFFF);
    prvTransmitHalfFrame(frame & 0xFFFF);
}

static void prvWriteHardware(uint32_t *data) {
    for (int i = 0; i < 3; i++) {
        prvTransmitFrame(data[2 - i]);
    }
    while (DL_SPI_isBusy(CLOCK_SPI_INST)) {
        taskYIELD();
    }
    DL_GPIO_setPins(MISC_GPIO_PORT, MISC_GPIO_CLOCK_LOAD_PIN);
    DL_TimerG_startCounter(LCD_DELAY_LOAD_INST);
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
    DL_GPIO_clearPins(MISC_GPIO_PORT, MISC_GPIO_CLOCK_LOAD_PIN);
}

void LCD_DELAY_LOAD_INST_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    switch (DL_TimerG_getPendingInterrupt(LCD_DELAY_LOAD_INST)) {
    case DL_TIMER_IIDX_ZERO:
        vTaskNotifyGiveFromISR(xClockTaskId, &xHigherPriorityTaskWoken);
        break;
    default:
        break;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void prvSetDigit(uint32_t *data, uint32_t digit_value,
                        uint8_t digit_offset) {
    data[digit_offset / 32] |= digit_value << (digit_offset % 32);
}

static void prvRenderColon(uint32_t *data, uint8_t offset) {
    data[offset / 32] |= 1 << (offset % 32);
}

#ifndef NDBUG
static void prvSetSegmentForAll(uint32_t *data, uint8_t segment_id) {
    uint32_t segment = segment_id < 7 ? SEGMENTS[segment_id] : 0;
    for (uint8_t i = 0; i < 12 && segment; i++) {
        prvSetDigit(data, segment, DIGITS[i / 2][i % 2]);
    }
}
static void prvRunTestSequence(uint8_t seconds_per_test) {
    Clock_Message message;
    uint32_t data[3] = {0};
    uint8_t current_segment = 0;
    uint8_t seconds_count = 0;
    while (1) {
        // Hijack the queue. Use clockmsg_writehw as a once-per-second clock.
        if (xQueueReceive(clockQueue, &message, portMAX_DELAY) == pdTRUE) {
            if (message.type == clockmsg_writehw) {
                if (seconds_count == 0) {
                    for (int i = 0; i < 3; i++) {
                        data[i] = 0;
                    }
                    switch (current_segment) {
                    default:
                        prvSetSegmentForAll(data, current_segment);
                        break;
                    case 7:
                        times_ms[0] = ((1 * 60 + 23) * 60 + 45) * 1000;
                        times_ms[1] = ((1 * 60 + 23) * 60 + 45) * 1000;
                        prvRenderTime(times_ms, data);
                        break;
                    case 8:
                        for (uint8_t i = 0; i < 12; i++) {
                            prvSetDigit(data, SEVENSEG_8, DIGITS[i / 2][i % 2]);
                        }
                        prvRenderColon(data, COL_1_1_OFFSET);
                        prvRenderColon(data, COL_1_2_OFFSET);
                        prvRenderColon(data, COL_2_1_OFFSET);
                        prvRenderColon(data, COL_2_2_OFFSET);
                        break;
                    case 9:
                        times_ms[0] = 0;
                        times_ms[1] = 0;
                        prvRenderTime(times_ms, data);
                        times_ms[0] = 10000;
                        times_ms[1] = 10000;
                        break;
                    case 10:
                        times_ms[0] = ((1 * 60 + 23) * 60 + 45) * 1000;
                        ;
                        times_ms[1] = ((54 * 60 + 32) * 60 + 10) * 1000;
                        ;
                        prvRenderTime(times_ms, data);
                        break;
                    }
                    prvWriteHardware(data);
                    current_segment = (current_segment + 1) % 11;
                }
                seconds_count = seconds_per_test == 0
                                    ? 0
                                    : (seconds_count + 1) % seconds_per_test;
            }
        }
    }
}
#endif

static void prvRenderDigitPair(uint8_t value, uint32_t *data,
                               uint8_t offset_ones, uint8_t offset_tens) {
    prvSetDigit(data, NUMBERS[value % 10], offset_ones);
    prvSetDigit(data, NUMBERS[value / 10], offset_tens);
}

static void prvRenderTime(uint32_t *times, uint32_t *data) {
    for (int i = 0; i < 3; i++) {
        data[i] = 0;
    }
    uint32_t t = times[0] / 1000;
    prvRenderDigitPair(t % 60, data, DIGITS[2][0], DIGITS[2][1]);
    // seconds -> minutes
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGITS[1][0], DIGITS[1][1]);
        prvRenderColon(data, COL_1_2_OFFSET);
    }
    // minutes -> hours
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGITS[0][0], DIGITS[0][1]);
        prvRenderColon(data, COL_1_1_OFFSET);
    }
    // Black's time next. TODO
    t = times[1] / 1000;
    prvRenderDigitPair(t % 60, data, DIGITS[5][0], DIGITS[5][1]);
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGITS[4][0], DIGITS[4][1]);
        prvRenderColon(data, COL_2_2_OFFSET);
    }
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGITS[3][0], DIGITS[3][1]);
        prvRenderColon(data, COL_2_1_OFFSET);
    }
}

BaseType_t xClock_Init(void) {
    uint32_t data[3] = {0};
    prvRenderColon(data, COL_1_1_OFFSET);
    prvRenderColon(data, COL_1_2_OFFSET);
    prvRenderColon(data, COL_2_1_OFFSET);
    prvRenderColon(data, COL_2_2_OFFSET);
    prvWriteHardware(data);
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
                prvRenderTime(times_ms, data);
                prvWriteHardware(data);
                break;
            }
            case clockmsg_set_turn:
                if (state == clock_paused_black || state == clock_paused_white) {
                    state = message.set_turn_black ? clock_paused_black : clock_paused_white;
                } else {
                    state = message.set_turn_black ? clock_black_turn : clock_white_turn;
                }
                break;
            case clockmsg_test_segments:
                state = clock_white_turn;
                prvRunTestSequence(message.seconds_per_test);
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