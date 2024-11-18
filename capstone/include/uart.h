#ifndef CAPSTONE_UART_H
#define CAPSTONE_UART_H

#include "config.h"
#include "game.h"
#include "uart_bidir_protocol.h"

BaseType_t xUART_init(void);

PieceType xPtypeFromWire(PTYPE in, BaseType_t whiteToMove);

#endif
