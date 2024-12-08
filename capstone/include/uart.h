#ifndef CAPSTONE_UART_H
#define CAPSTONE_UART_H

#include "config.h"
#include "game.h"
#include "uart_bidir_protocol.h"

#define HEARTBEAT_TIME_MS (100)
#define MSP_SYN (0x00000000ul)
#define SYNACK  (0x0000FFFFul)
#define MSP_ACK (0xFFFFFFFFul)

#define SENTINEL_CHECKMATE (0x00000031ul)
#define SENTINEL_STALEMATE (0x00000071ul)
#define SENTINEL_REQUEST_RESEND_MOVES (0x00000004ul)

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
void vUART_SendCalibration(uint16_t min, uint16_t max, uint8_t row, uint8_t col, PieceType type);

PieceType xPtypeFromWire(PTYPE in, BaseType_t whiteToMove);
PTYPE xPtypeToWire(PieceType in);

void vUART_Task(void *arg0);
#endif
