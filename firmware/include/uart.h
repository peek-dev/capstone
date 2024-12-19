/**
 * Copyright (C) 2024 John E. Berberian, Jr., and Paul D. Karhnak
 *
 * uart.h: The core interface definition for MSPM0G3507 communication with the 
 * Raspberry Pi over UART, including interface constants and UART data movement
 * (FIFO enqueue/dequeue) functions.
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
#ifndef CAPSTONE_UART_H
#define CAPSTONE_UART_H

#include "config.h"
#include "game.h"
#include "uart_bidir_protocol.h"

#define HEARTBEAT_TIME_MS (100)
#define MSP_SYN           (0x00000008ul)
#define SYNACK            (0x00000001ul)
#define MSP_ACK           (0xFFFFFFFFul)

#define SENTINEL_CHECKMATE            (0x00000031ul)
#define SENTINEL_STALEMATE            (0x00000071ul)
#define SENTINEL_REQUEST_RESEND_MOVES (0x00000004ul)
#define SENTINEL_UNDO_EXHAUSTED       (0x00000008ul)

/**
 * Initialize UART thread state, including queue handle for data to be put on
 * the wire.
 */
BaseType_t xUART_Init(void);

/**
 * Enqueue data to be transmitted over the MSP UART TX to the Raspberry Pi.
 */
BaseType_t xUART_to_wire(uint32_t move);
BaseType_t xUART_EncodeEvent(BUTTON_EVENT button, NormalMove move);
void vUART_SendCalibration(uint16_t min, uint16_t max, uint8_t row, uint8_t col,
                           PieceType type);

PieceType xPtypeFromWire(PTYPE in, BaseType_t whiteToMove);
PTYPE xPtypeToWire(PieceType in);

void vUART_Task(void *arg0);
#endif
