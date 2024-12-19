/**
 * Copyright (C) <year>  <name of author>
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
#include "uart.h"
#include "config.h"
#include "game.h"
#include "projdefs.h"
#include "portmacro.h"
#include "ti_msp_dl_config.h"
#include <queue.h>
#include "main.h"
#include "uart_bidir_protocol.h"
#include "assert.h"
#include "sensor_mutex.h"

#define UART_QUEUE_SIZE 12

// RPI_UART_INST

// The "private" queue handle for data to be placed on UART TX.
static QueueHandle_t queue_to_wire;

// For blocking/unblocking from interrupts.
static TaskHandle_t xUARTTaskId = NULL;

static void prvUART_UnpackAndSend(uint32_t packet);

/**
 * Initialize UART thread state, including queue handle for data to be put on
 * the wire.
 */
BaseType_t xUART_Init(void) {
    queue_to_wire = xQueueCreate(UART_QUEUE_SIZE, sizeof(uint32_t));

    if (queue_to_wire == NULL) {
        return pdFALSE;
    }

    NVIC_ClearPendingIRQ(RPI_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(RPI_UART_INST_INT_IRQN);

    return pdTRUE;
}

/**
 * Enqueue data to be transmitted over the MSP UART TX to the Raspberry Pi.
 *
 * Callers should be in other threads (e.g., main)
 */
BaseType_t xUART_to_wire(uint32_t move) {
    return xQueueSend(queue_to_wire, &move, portMAX_DELAY);
}

BaseType_t xUART_EncodeEvent(BUTTON_EVENT button, NormalMove move) {
    uint32_t request = button;
    if (button == BUTTON_TURNSWITCH) {
        // This works, modulo a bunch of dontcares.
        request = move;
        request = (request & (~0x3)) | button;
    }
    return xUART_to_wire(request);
}

void vUART_SendCalibration(uint16_t min, uint16_t max, uint8_t row, uint8_t col,
                           PieceType type) {
    BaseType_t xReturned;
    uint32_t word = (max << 4) | M2_MASK;
    word |= xPtypeToWire(type) << 0;
    word |= (type >= BlackKing) << 3;
    word |= row << SRC_RANK_SHIFT;
    word |= col << SRC_FILE_SHIFT;

    xReturned = xUART_to_wire(word);
    while (xReturned != pdPASS);

    word = min << 4;
    word |= xPtypeToWire(type) << 0;
    word |= (type >= BlackKing) << 3;
    word |= row << SRC_RANK_SHIFT;
    word |= col << SRC_FILE_SHIFT;
    xReturned = xUART_to_wire(word);
    while (xReturned != pdPASS);
}

PTYPE xPtypeToWire(PieceType in) {
    switch (in) {
    case BlackPawn:
    case WhitePawn:
        return PTYPE_PAWN;
    case BlackBishop:
    case WhiteBishop:
        return PTYPE_BISHOP;
    case BlackRook:
    case WhiteRook:
        return PTYPE_ROOK;
    case BlackKnight:
    case WhiteKnight:
        return PTYPE_KNIGHT;
    case BlackKing:
    case WhiteKing:
        return PTYPE_KING;
    case BlackQueen:
    case WhiteQueen:
        return PTYPE_QUEEN;
    case EmptySquare:
        return PTYPE_NULL_ALT;
    }
}
PieceType xPtypeFromWire(PTYPE in, BaseType_t white) {
    PieceType result;
    // Start out with black pieces.
    switch (in) {
    case PTYPE_NULL:
    case PTYPE_NULL_ALT:
        result = EmptySquare;
        break;
    case PTYPE_PAWN:
        result = BlackPawn;
        break;
    case PTYPE_KNIGHT:
        result = BlackKnight;
        break;
    case PTYPE_BISHOP:
        result = BlackBishop;
        break;
    case PTYPE_ROOK:
        result = BlackRook;
        break;
    case PTYPE_QUEEN:
        result = BlackQueen;
        break;
    case PTYPE_KING:
        result = BlackKing;
        break;
    default:
        result = EmptySquare;
    }
    // If the piece is white, swap the color.
    if (white == pdTRUE) {
        result = xChangeColor(result);
    }
    return result;
}

/**
 * "Break up" the 32-bit word into 8-bit packets to send (UART capacity is
 * 8 data bits)
 */
static void prvUART_UnpackAndSend(uint32_t packet) {
    uint8_t next_byte;
#if UART_USE_SENSOR_MUTEX
    xSemaphoreTake(sensor_mutex, portMAX_DELAY);
#endif
    for (int i = 0; i < 4; i += 1) {
        next_byte = (uint8_t)(packet & 0xFF);
        // Try to transmit.
        while (!DL_UART_transmitDataCheck(RPI_UART_INST, next_byte)) {
            // If the fifo is full, wait for up to 3ms for the fifo to clear
            // out. An interrupt will wake us up when it's empty.
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3));
        }
        packet >>= 8;
    }
#if UART_USE_SENSOR_MUTEX
    xSemaphoreGive(sensor_mutex);
#endif
}

void RPI_UART_INST_IRQHandler(void) {
    static uint8_t shift = 0;
    static uint32_t packet = 0;
    static BaseType_t got_sync = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    switch (DL_UART_getPendingInterrupt(RPI_UART_INST)) {
    case DL_UART_IIDX_RX:
        if (got_sync == pdFALSE) {
            packet = (uint32_t)DL_UART_receiveDataBlocking(RPI_UART_INST);
            if (packet == (SYNACK & 0xFF)) {
                got_sync = pdTRUE;
                shift += 8;
                packet |= ((uint32_t)DL_UART_receiveDataBlocking(RPI_UART_INST))
                          << shift;
                shift += 8;
            }
            break;
        }
        // There are at least two chunks waiting for us. This will not block.
        // If there were fewer than two chunks, the interrupt wouldn't fire.
        packet |= ((uint32_t)DL_UART_receiveDataBlocking(RPI_UART_INST))
                  << shift;
        shift += 8;
        packet |= ((uint32_t)DL_UART_receiveDataBlocking(RPI_UART_INST))
                  << shift;
        shift += 8;
        if (shift == 32) {
            shift = 0;
            xMain_uart_message_FromISR(packet, &xHigherPriorityTaskWoken);
            packet = 0;
        }
        break;
    case DL_UART_IIDX_TX:
        // Only actually do this if the thread exists...
        if (xUARTTaskId != NULL) {
            // The TX hardware fifo is empty. Wake up the task if it's blocked.
            vTaskNotifyGiveFromISR(xUARTTaskId, &xHigherPriorityTaskWoken);
        }
        break;
    default:
        break;
    }

    // If we woke up a higher-priority task than the one currently running,
    // we should let the scheduler know so that it can switch.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * The main loop for the UART task.
 */
void vUART_Task(void *arg0) {
    assert(xUARTTaskId == NULL);
    xUARTTaskId = xTaskGetCurrentTaskHandle();

    /* Thread-local variable for sending words */
    uint32_t next_sent;

    while (1) {
        if (xQueueReceive(queue_to_wire, &next_sent, portMAX_DELAY) == pdPASS) {
            prvUART_UnpackAndSend(next_sent);
        }
    }
}
