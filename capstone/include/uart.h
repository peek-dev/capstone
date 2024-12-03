#ifndef CAPSTONE_UART_H
#define CAPSTONE_UART_H

#include "config.h"
#include "game.h"
#include "uart_bidir_protocol.h"

/**
 * Initialize UART thread state, including queue handle for data to be put on 
 * the wire.
 */
BaseType_t xUART_init(void);

/**
 * Enqueue data to be transmitted over the MSP UART TX to the Raspberry Pi.
 */
BaseType_t xUART_to_wire(uint32_t move);
BaseType_t xUART_EncodeEvent(BUTTON_EVENT button, NormalMove move);

PieceType xPtypeFromWire(PTYPE in, BaseType_t whiteToMove);
void vUART_Task(void *arg0);
#endif
