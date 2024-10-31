#include "config.h"

#include <string.h>
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
    clockmsg_turnswitch,
};

typedef struct {
    // TODO: check that this is only using 8 bits, not more.
    enum Clock_MsgType type;
    uint32_t *times_ptr;
} Clock_Message;
static TaskHandle_t xClockTaskId = NULL;

void vApplicationTickHook(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // If the tick rate doesn't evenly divide 1000, you'll be in for a world of
    // trouble. Please make the tick rate reasonable.
    if (state == clock_white_turn || state == clock_black_turn) {
        // Abuse enums being numbers to index.
        times_ms[state] -= (1000 / configTICK_RATE_HZ);
        // If we've just finished a second, wakeup the thread to signal the LCD.
        if (times_ms[state] % 1000 == 0) {
            Clock_Message msg = {.type = clockmsg_writehw, .times_ptr = NULL};
            xQueueSendFromISR(clockQueue, &msg, &xHigherPriorityTaskWoken);
        }
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void prvTransmitFrame(uint32_t frame) {
    // My own reimplementation of DL_SPI_transmitDataBlocking32,
    // but using yield instead of busy-wait.
    while (DL_SPI_isTXFIFOFull(CLOCK_SPI_INST)) {
        taskYIELD();
    }
    DL_SPI_transmitData32(CLOCK_SPI_INST, frame);
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

static uint32_t prvSevenSegmentDigit(char c) {
    switch (c) {
    case '0':
        return CLOCK_0;
        break;
    case '1':
        return CLOCK_1;
        break;
    case '2':
        return CLOCK_2;
        break;
    case '3':
        return CLOCK_3;
        break;
    case '4':
        return CLOCK_4;
        break;
    case '5':
        return CLOCK_5;
        break;
    case '6':
        return CLOCK_6;
        break;
    case '7':
        return CLOCK_7;
        break;
    case '8':
        return CLOCK_8;
        break;
    case '9':
        return CLOCK_9;
        break;
    case 'U':
    case 'u':
        return CLOCK_U;
        break;
    case 'N':
    case 'n':
        return CLOCK_N;
        break;
    case 'D':
    case 'd':
        return CLOCK_D;
        break;
    case 'O':
    case 'o':
        return CLOCK_O;
        break;
    default:
        return 0;
        break;
    }
}

static void prvRenderDigitPair(uint8_t value, uint32_t *data,
                               uint8_t offset_ones, uint8_t offset_tens) {
    data[offset_ones / 32] |= prvSevenSegmentDigit('0' + (value % 10))
                              << (offset_ones % 32);
    data[offset_tens / 32] |= prvSevenSegmentDigit('0' + (value / 10))
                              << (offset_tens % 32);
}

static void prvRenderColon(uint32_t *data, uint8_t offset) {
    data[offset / 32] |= 1 << (offset % 32);
}

static void prvRenderTime(uint32_t *times, uint32_t *data) {
    for (int i = 0; i < 3; i++) {
        data[i] = 0;
    }
    uint32_t t = times[0] / 1000;
    prvRenderDigitPair(t % 60, data, DIGIT_1_6_OFFSET, DIGIT_1_5_OFFSET);
    // seconds -> minutes
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGIT_1_4_OFFSET, DIGIT_1_3_OFFSET);
        prvRenderColon(data, COL_1_2_OFFSET);
    }
    // minutes -> hours
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGIT_1_2_OFFSET, DIGIT_1_1_OFFSET);
        prvRenderColon(data, COL_1_1_OFFSET);
    }
    // Black's time next. TODO
    t = times[1] / 1000;
    prvRenderDigitPair(t % 60, data, DIGIT_2_6_OFFSET, DIGIT_2_5_OFFSET);
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGIT_2_4_OFFSET, DIGIT_2_3_OFFSET);
        prvRenderColon(data, COL_2_2_OFFSET);
    }
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGIT_2_2_OFFSET, DIGIT_2_1_OFFSET);
        prvRenderColon(data, COL_2_1_OFFSET);
    }
    for (int i = 0; i < 3; i++) {
        data[i] = (data[i] >> 16) | (data[i] << 16);
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
                memcpy(times_ms, message.times_ptr, 2 * sizeof(uint32_t));
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
            case clockmsg_turnswitch:
                if (state == clock_white_turn) {
                    state = clock_black_turn;
                } else if (state == clock_black_turn) {
                    state = clock_white_turn;
                }
                break;
            }
        }
    }
}