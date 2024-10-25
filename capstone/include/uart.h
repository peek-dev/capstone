#ifndef CAPSTONE_UART_H
#define CAPSTONE_UART_H

#include "game.h"
#include "portmacro.h"
#include "uart_bidir_protocol.h"

PieceType xPtypeFromWire(PTYPE in, BaseType_t whiteToMove);

#endif