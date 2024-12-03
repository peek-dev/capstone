#include "uart.h"
#include "config.h"
#include "game.h"
#include "projdefs.h"
#include "portmacro.h"
#include "ti_msp_dl_config.h"
#include <queue.h>
#include "main.h"
#include "uart_bidir_protocol.h"
#define UART_QUEUE_SIZE 12

// RPI_UART_INST

// The "private" queue handle for data to be placed on UART TX.
static QueueHandle_t queue_to_wire;

/**
 * Initialize UART thread state, including queue handle for data to be put on
 * the wire.
 */
BaseType_t xUART_init(void) {
    queue_to_wire = xQueueCreate(UART_QUEUE_SIZE, sizeof(uint32_t));

    if (queue_to_wire == NULL) {
        return pdFALSE;
    }

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
static void prvUART_UnpackAndSend(UART_Regs *uart, uint32_t packet) {
    uint8_t next_byte;

    for (int i = 0; i < 4; i += 1) {
        next_byte = (uint8_t)(packet & 0xFF);
        DL_UART_transmitDataBlocking(uart, next_byte);
        packet >>= 8;
    }
}

/**
 * Do the reverse of sending: collect 4 bytes at a time into a single protocol
 * word.
 */
static uint32_t prvUART_PackAndReceive(UART_Regs *uart) {
    uint32_t retrieved_word = 0;
    uint8_t next_byte;

    for (int i = 0; i < 4; i += 1) {
        retrieved_word <<= 8;
        next_byte = DL_UART_receiveDataBlocking(uart);
        retrieved_word |= (((uint32_t)next_byte) & 0xFF);
    }

    return retrieved_word;
}

/**
 * The main loop for the UART task.
 */
void vUART_Task(void *arg0) {

    /* Thread-local variables for sending and receiving words */
    uint32_t next_sent;
    uint32_t next_packet;

    while (1) {
        BaseType_t available_from_main = uxQueueMessagesWaiting(queue_to_wire);

        if (available_from_main > 0) {
            // Send all ready data from main to the Raspberry Pi
            for (BaseType_t i = 0; i < available_from_main; i += 1) {
                next_sent =
                    xQueueReceive(queue_to_wire, &next_sent, portMAX_DELAY);
                prvUART_UnpackAndSend(RPI_UART_INST, next_sent);
            }
        }

        do {
            next_packet = prvUART_PackAndReceive(RPI_UART_INST);
            xMain_uart_message(next_packet);
        } while (!IS_LAST_MOVE(
            next_packet)); // While the last packet received is **not** the last
                           // packet in this series

        taskYIELD(); // Rather than busy wait on FIFOs, yield to other tasks
    }
}
