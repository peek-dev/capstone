#include "uart.h"
#include "config.h"
#include "game.h"
#include "projdefs.h"

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

static void prvUART_UnpackAndSend(UART_Regs* uart, uint32_t packet) {
    uint8_t next_byte;

    for (int i = 0; i < 4; i += 1) {
        next_byte = (uint8_t) (packet & 0xFF);
        DL_UART_transmitDataBlocking(uart, next_byte);
        packet >>= 8;
    }
}

static uint32_t prvUART_PackAndReceive(UART_Regs* uart) {
    uint32_t retrieved_word = 0;
    uint8_t next_byte;

    for (int i = 0; i < 4; i += 1) {
        retrieved_word <<= 8;
        next_byte = DL_UART_receiveDataBlocking(uart);
        retrieved_word |= (((uint32_t) next_byte) & 0xFF);
    }

    return retrieved_word;
}

void vUART_Task(void* arg0) {
    UART_arg* arg = (UART_arg*) arg0;
    UART_Regs* uart; // TODO: define based on sysconfig (?)

    DL_UART_Config uart_cfg = { 
        .mode = DL_UART_MODE_NORMAL, /* blocking I/O */
        .direction = DL_UART_TX_RX, /* duplex communication */
        .flowControl = DL_UART_FLOW_CONTROL_NONE,
        /* "Default" UART params: 1 start + 8 data + 1 stop (no parity) */
        .parity = DL_UART_PARITY_NONE,
        .wordLength = DL_UART_WORD_LENGTH_8_BITS,
        .stopBits = DL_UART_STOP_BITS_ONE
    };

    while (1) { 
        // Block on the main-to-UART semaphore to wait for new input
        xSemaphoreTake(arg->mtu_lock, portMAX_DELAY);
        BaseType_t available_from_main = uxQueueMessagesWaiting(arg->main_to_uart);

        if (available_from_main != 0) {
            uint32_t from_main[available_from_main];

            for (BaseType_t i = 0; i < available_from_main; i += 1) {
                xQueueReceive(arg->main_to_uart, &(from_main[i]), 0);
            }

            // Usage all done, so free up queue
            xSemaphoreGive(arg->mtu_lock);

            // Send all ready data from main to the Raspberry Pi
            for (BaseType_t i = 0; i < available_from_main; i += 1) {
                prvUART_UnpackAndSend(uart, from_main[i]);
            }
        } else {
            xSemaphoreGive(arg->mtu_lock); // No data ready yet
        }

        // TODO: declare queue for received messages from UART?
        
        uint32_t next_packet;

        do {
            next_packet = prvUART_PackAndReceive(uart);
            // TODO: enqueue packet on thread-local FIFO
            // TODO: add logic to check if overflowing thread-local FIFO.
            //          if so, add logic to en masse dequeue and enqueue data onto the UART->main queue
        } while (!(next_packet & 1)); // While the last packet received is **not** the last packet in this series

        taskYIELD(); // Rather than busy wait on FIFOs, yield to other tasks
    }

}