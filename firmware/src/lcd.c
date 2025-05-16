/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * lcd.c: driver for our seven-segment display SPI hardware (AY0438).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */
#include "config.h"
#include "game.h"
#include <task.h>
#include <queue.h>
#include "sensor_mutex.h"
#include "clock.h"
#include "clock_private.h"
#include "lcd.h"

extern TaskHandle_t xClockTaskId;
extern QueueHandle_t clockQueue;

#define TURN_OFFSET(turn) ((turn == game_turn_black) ? 0 : 3)

// [left, right], each element [ones, tens]
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

static void prvSetDigit(uint32_t *data, uint32_t digit_value,
                        uint8_t digit_offset) {
    data[digit_offset / 32] |= digit_value << (digit_offset % 32);
}

static void prvRenderColon(uint32_t *data, uint8_t offset) {
    data[offset / 32] |= 1 << (offset % 32);
}

static void prvRenderDigitPair(uint8_t value, uint32_t *data,
                               uint8_t offset_ones, uint8_t offset_tens) {
    prvSetDigit(data, NUMBERS[value % 10], offset_ones);
    prvSetDigit(data, NUMBERS[value / 10], offset_tens);
}

static void prvSetSegmentForAll(uint32_t *data, uint8_t segment_id) {
    uint32_t segment = segment_id < 7 ? SEGMENTS[segment_id] : 0;
    for (uint8_t i = 0; i < 12 && segment; i++) {
        prvSetDigit(data, segment, DIGITS[i / 2][i % 2]);
    }
}

void prvRenderTime_oneside(uint32_t t, uint32_t *data, game_turn side) {
    uint8_t offset = TURN_OFFSET(side);
    prvRenderDigitPair(t % 60, data, DIGITS[2 + offset][0],
                       DIGITS[2 + offset][1]);
    // seconds -> minutes
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGITS[1 + offset][0],
                           DIGITS[1 + offset][1]);
        prvRenderColon(data, (offset == 3) ? COL_2_2_OFFSET : COL_1_2_OFFSET);
    }
    // minutes -> hours
    t /= 60;
    if (t > 0) {
        prvRenderDigitPair(t % 60, data, DIGITS[0 + offset][0],
                           DIGITS[0 + offset][1]);
        prvRenderColon(data, (offset == 3) ? COL_2_1_OFFSET : COL_1_1_OFFSET);
    }
}

static void prvRenderTime(uint32_t *times, uint32_t *data) {
    // Start with white's time.
    prvRenderTime_oneside(times[0] / 1000, data, game_turn_white);
    // Black's time next.
    prvRenderTime_oneside(times[1] / 1000, data, game_turn_black);
}

static void prvRenderPause(uint32_t *data, game_turn turn, uint32_t *times) {
    prvRenderTime_oneside(times[turn] / 1000, data, turn);
    // This is the reverse of the usual offset.
    uint8_t offset = 3 - TURN_OFFSET(turn);
    prvSetDigit(data, CLOCK_P, DIGITS[0 + offset][0]);
    prvSetDigit(data, CLOCK_A, DIGITS[1 + offset][1]);
    prvSetDigit(data, CLOCK_U, DIGITS[1 + offset][0]);
    prvSetDigit(data, CLOCK_S, DIGITS[2 + offset][1]);
    prvSetDigit(data, CLOCK_E, DIGITS[2 + offset][0]);
}

void prvRenderNumbers_oneside(uint16_t n, uint32_t *data, game_turn side) {
    uint8_t offset = TURN_OFFSET(side);
    for (uint8_t i = 0; i < 6 && (n != 0 || i == 0); i++) {
        prvSetDigit(data, NUMBERS[n % 10], DIGITS[offset + 2 - (i / 2)][i % 2]);
        n /= 10;
    }
}

static void prvRenderUndo(uint32_t *data, game_turn turn, uint16_t *numbers) {
    prvRenderNumbers_oneside(numbers[1 - turn], data, 1 - turn);
    uint8_t offset = TURN_OFFSET(turn);
    prvSetDigit(data, CLOCK_U, DIGITS[1 + offset][1]);
    prvSetDigit(data, CLOCK_N, DIGITS[1 + offset][0]);
    prvSetDigit(data, CLOCK_D, DIGITS[2 + offset][1]);
    prvSetDigit(data, CLOCK_o, DIGITS[2 + offset][0]);
}

static void prvRenderNumbers(uint32_t *data, uint16_t *numbers) {
    prvRenderNumbers_oneside(numbers[0], data, game_turn_white);
    prvRenderNumbers_oneside(numbers[1], data, game_turn_black);
}

static void prvRenderColons(uint32_t *data) {
    prvRenderColon(data, COL_1_1_OFFSET);
    prvRenderColon(data, COL_1_2_OFFSET);
    prvRenderColon(data, COL_2_1_OFFSET);
    prvRenderColon(data, COL_2_2_OFFSET);
}

void vLCD_RenderState(uint32_t *data, clock_state state, game_turn turn,
                      uint32_t *times_ms, uint16_t *numbers, uint32_t inc) {
    for (uint8_t i = 0; i < 3; i++) {
        data[i] = 0;
    }
    switch (state) {
    case clock_state_off:
        prvRenderColons(data);
        break;
    case clock_state_notstarted:
        prvRenderTime_oneside(times_ms[0] / 1000, data, game_turn_black);
        prvRenderNumbers_oneside(inc / 1000, data, game_turn_white);
        break;
    case clock_state_running:
        // Render the current times.
        prvRenderTime(times_ms, data);
        break;
    case clock_state_paused:
        prvRenderPause(data, turn, times_ms);
        break;
    case clock_state_staticnumbers:
        prvRenderNumbers(data, numbers);
        break;
    case clock_state_undo:
        prvRenderUndo(data, turn, numbers);
        break;
    }
}

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

void vLCD_WriteHardware(uint32_t *data) {
#if LCD_USE_SENSOR_MUTEX
    xSemaphoreTake(sensor_mutex, portMAX_DELAY);
#endif
    for (int i = 0; i < 3; i++) {
        prvTransmitFrame(data[2 - i]);
    }
    while (DL_SPI_isBusy(CLOCK_SPI_INST)) {
        taskYIELD();
    }
    DL_GPIO_setPins(MISC_GPIO_CLOCK_LOAD_PORT, MISC_GPIO_CLOCK_LOAD_PIN);
    DL_TimerG_startCounter(LCD_DELAY_LOAD_INST);
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
    DL_GPIO_clearPins(MISC_GPIO_CLOCK_LOAD_PORT, MISC_GPIO_CLOCK_LOAD_PIN);
#if LCD_USE_SENSOR_MUTEX
    xSemaphoreGive(sensor_mutex);
#endif
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

void vLCD_Init(void) {
    uint32_t data[3] = {0};
    prvRenderColon(data, COL_1_2_OFFSET);
    prvRenderColon(data, COL_2_1_OFFSET);
    vLCD_WriteHardware(data);
    NVIC_ClearPendingIRQ(LCD_DELAY_LOAD_INST_INT_IRQN);
    NVIC_EnableIRQ(LCD_DELAY_LOAD_INST_INT_IRQN);
}

void vLCD_RunTestSequence(uint8_t seconds_per_test) {
    Clock_Message message;
    uint32_t data[3] = {0};
    uint32_t times_ms[2];
    uint8_t current_segment = 0;
    uint8_t seconds_count = 0;
    while (1) {
        // Hijack the queue. Use clockmsg_writehw as a once-per-second clock.
        if (xQueueReceive(clockQueue, &message, portMAX_DELAY) == pdTRUE) {
            if (message.type == clockmsg_render_state) {
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
                    vLCD_WriteHardware(data);
                    current_segment = (current_segment + 1) % 11;
                }
                seconds_count = seconds_per_test == 0
                                    ? 0
                                    : (seconds_count + 1) % seconds_per_test;
            }
        }
    }
}
