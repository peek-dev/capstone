/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * main_interactive_sensors.c: alternative event loop to display the
 * detected piece type on a selected square.
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
#include <task.h>
#include <string.h>
#include <queue.h>

#include "chess.h"
#include "game.h"
#include "clock.h"
#include "led.h"
#include "portmacro.h"
#include "projdefs.h"
#include "sensor.h"
#include "uart_bidir_protocol.h"
#include "led_translation.h"
#include "button.h"
#include "sensor_mutex.h"
#include "calibration.h"
#include "util.h"

#define DECLARE_PRIVATE_MAIN_C
#include "main.h"

#define SENSOR_DELAY_MS 20
// Based on our filter step responses, we should wait 150us for a column switch.
// (150 us * 32 MHz) - 1
#define COL_SWITCH_LOAD (150U * 64U / 10U - 1U)
// Similarly, 50us for a row switch.
#define ROW_SWITCH_LOAD (50U * 64U / 10U - 1U)

SemaphoreHandle_t sensor_mutex;
void vPeriodicPing_Thread(void *arg0);

void mainThread(void *arg0) {
    /* FIXME: declare task handle for UART task */
    TaskHandle_t thread_led, thread_pings;
    BaseType_t xReturned;
    sensor_mutex = xSemaphoreCreateMutex();

    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {}
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {}
    xReturned = xMain_Init();
    while (xReturned != pdPASS) {}
    xReturned = xSensor_Init();
    while (xReturned != pdPASS) {}
    vButton_Init();

    xReturned = xPortGetFreeHeapSize();

    xReturned = xTaskCreate(vLED_Thread, "LED", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_led);
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vPeriodicPing_Thread, "Pings", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_pings);
    while (xReturned != pdPASS) {}

    MainThread_Message message;
    MAKEVISIBLE BaseType_t mem = xPortGetFreeHeapSize();
    while (1) {
        if (xQueueReceive(mainQueue, &message, portMAX_DELAY) == pdTRUE) {
            prvProcessMessage(&message);
        }
    }

    vTaskDelete(NULL);
}

extern TaskHandle_t xSensorTaskId;
BaseType_t xMain_Init(void) {
    xSensorTaskId = xTaskGetCurrentTaskHandle();
    mainQueue = xQueueCreate(QUEUE_SIZE, sizeof(MainThread_Message));
    if (mainQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

BaseType_t xMain_sensor_update(BoardState *state) {
    MainThread_Message m;
    m.type = main_sensor_update;
    memcpy(&m.state, state, sizeof(BoardState));
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

BaseType_t xMain_button_press_FromISR(enum button_num button,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    MainThread_Message m;
    m.type = main_button_press;
    m.button = button;
    return xQueueSendFromISR(mainQueue, &m, pxHigherPriorityTaskWoken);
}

BaseType_t xMain_uart_message(uint32_t move) {
    MainThread_Message m;
    m.type = main_uart_message;
    m.move = move;
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

void prvSelectRow(uint8_t row);
void prvSelectColumn(uint8_t column);
static void prvHandleButtonPress(enum button_num button) {
    // If it's the turn-switch button:
    uint16_t previousMovesLen = prvMovesLen;
    switch (button) {
    case button_num_white_move:
        prvMovesLen = (prvMovesLen + 63) % 64;
        break;
    case button_num_black_move:
        prvMovesLen = (prvMovesLen + 1) % 64;
        break;
    default:
        break;
    }
    if (prvMovesLen % 8 != previousMovesLen % 8) {
        // Set the timer for 150us delay.
        DL_TimerG_setLoadValue(SENSOR_DELAY_TIMER_INST, COL_SWITCH_LOAD);
        prvSelectColumn(prvMovesLen % 8);
        // Wait for the timer, at most waiting 1ms.
        DL_TimerG_startCounter(SENSOR_DELAY_TIMER_INST);
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
    }
    if (prvMovesLen / 8 != previousMovesLen / 8) {
        DL_TimerG_setLoadValue(SENSOR_DELAY_TIMER_INST, ROW_SWITCH_LOAD);
        prvSelectRow(prvMovesLen / 8);
        DL_TimerG_startCounter(SENSOR_DELAY_TIMER_INST);
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
    }
}

uint16_t prvSingleADC(void);

static void prvRenderState(void) {
    uint16_t samples[5];
    uint8_t row = prvMovesLen / 8;
    uint8_t col = prvMovesLen % 8;
    for (uint8_t i = 0; i < 5; i++) {
        samples[i] = prvSingleADC();
    }
    uint16_t sample = MedianOfFive(samples);
    PieceType piece = xValueToPiece(sample, row, col);
    Color color = {.brightness = 31, .red = 255, .green = 255, .blue = 255};
    // Illuminate the square and the piece type on that square.
    xLED_clear_board();
    xLED_set_color(LEDTrans_Square(row, col), &color);
    ZeroToTwoInts pieceOutlines = LEDTrans_Ptype(piece);
    for (uint8_t i = 0; i < pieceOutlines.len; i++) {
        xLED_set_color(pieceOutlines.data[i], &color);
    }
    xLED_commit();
}

static void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case main_uart_message:
        prvRenderState();
        break;
    case main_button_press:
        prvHandleButtonPress(message->button);
        // Self-trigger a sensor read.
        xMain_uart_message(0);
        break;
    default:
        break;
    }
}

BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state) {
    return pdTRUE;
}

BaseType_t xMain_uart_message_FromISR(uint32_t move,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    return pdTRUE;
}

void vPeriodicPing_Thread(void *arg0) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(SENSOR_DELAY_MS));
        // Ping the main thread. This will be interpreted as a request for a new sample.
        xMain_uart_message(0);
    }
    vTaskDelete(NULL);
}