/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * TODO: add one-sentence description of what this does
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
#include "uart.h"
#include "uart_bidir_protocol.h"
#include "button.h"

#define DECLARE_PRIVATE_MAIN_C
#include "main.h"

void mainThread(void *arg0) {
    MainThread_Message message;
    BaseType_t xReturned;

    xReturned = xMain_Init();
    while (xReturned != pdPASS) {}

    xReturned = xUART_Init();
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vUART_Task, "UART", configMINIMAL_STACK_SIZE, NULL,
                            3, &thread_uart);
    while (xReturned != pdPASS) {}

    // Wait until we get the proper synack packet from the RPi.
    // If we've waited more than HEARTBEAT_TIME_MS, try again.
    while (xQueueReceive(mainQueue, &message,
                         HEARTBEAT_TIME_MS / portTICK_PERIOD_MS) == pdFAIL ||
           message.move != SYNACK) {
        // Send our syn packet.
        xReturned = xUART_to_wire(MSP_SYN);
        while (xReturned != pdPASS);
    }
    // Okay, we've gotten the right synack packet. Cool.
    // Send the ack after we've initialized all our hardware and threads.

    xReturned = xUART_to_wire(MSP_ACK);
    while (xReturned != pdPASS);

    MAKEVISIBLE BaseType_t mem = xPortGetFreeHeapSize();
    while (1) {
        if (xQueueReceive(mainQueue, &message, portMAX_DELAY) == pdTRUE) {
            prvProcessMessage(&message);
        }
    }

    vTaskDelete(NULL);
}

SemaphoreHandle_t sensor_mutex;
BaseType_t xMain_Init(void) {
    sensor_mutex = xSemaphoreCreateMutex();
    mainQueue = xQueueCreate(QUEUE_SIZE, sizeof(MainThread_Message));
    state.turn = game_turn_white;
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

BaseType_t xMain_uart_message_FromISR(uint32_t move,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    MainThread_Message m;
    m.type = main_uart_message;
    m.move = move;
    return xQueueSendFromISR(mainQueue, &m, pxHigherPriorityTaskWoken);
}

static void prvSwitchStateTurn(GameState *statevar) {
    if (statevar->turn == game_turn_black) {
        statevar->turn = game_turn_white;
    } else {
        statevar->turn = game_turn_black;
    }
}

static void prvSwitchTurnRoutine() {
    // If we haven't finished getting possible moves yet, ignore the request.
    if (prvMovesLen == 0) {
        return;
    }
    // if it passes, proceed with switching turns.
    // Reset the possible moves.
    prvMovesLen = 0;
    prvCurrentMoveIndex = 0;
    BaseType_t xReturned =
        xUART_EncodeEvent(BUTTON_TURNSWITCH, prvMoves.possible[0]);
    while (xReturned != pdPASS);
    // Switch the player turn.
    prvSwitchStateTurn(&state);
}

static void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case main_sensor_update:
        break;
    case main_uart_message:
        // Check: are we currently listening for moves?
        if (prvMovesLen == 0) {
            prvMoves.possible[prvCurrentMoveIndex] = message->move;
            prvCurrentMoveIndex++;
            if (IS_LAST_MOVE(message->move)) {
                prvMovesLen = prvCurrentMoveIndex;
                // Movable pieces will be rendered on the next sensor input.
                // vTaskDelay(pdMS_TO_TICKS(1000));
                prvSwitchTurnRoutine();
            }
        }
        break;
    case main_button_press:
        break;
    }
}
BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state) {
    return pdTRUE;
}
