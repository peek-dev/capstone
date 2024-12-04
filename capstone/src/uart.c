#include "uart.h"
#include "config.h"
#include "game.h"
#include "projdefs.h"
#include "portmacro.h"
#include "ti_msp_dl_config.h"
#include <queue.h>
#include <ti/driverlib/dl_uart_main.h>
#include "main.h"

#ifndef QUEUE_SIZE
#define QUEUE_SIZE 16
#endif

// The "private" queue handle for data to be placed on UART TX.
static QueueHandle_t queue_to_wire; 

static void prvUART_UnpackAndSend(UART_Regs* uart, uint32_t packet);
static uint32_t prvUART_PackAndReceive(UART_Regs* uart);
static void prvUART_EstablishHeartbeat(void);

/**
 * Initialize UART thread state, including queue handle for data to be put on 
 * the wire.
 */
BaseType_t xUART_Init(void) {
    queue_to_wire = xQueueCreate(QUEUE_SIZE, sizeof(uint32_t));

    if (queue_to_wire == NULL) {
        return pdFALSE;
    }

    return pdTRUE;
}

/**
 * Enqueue data to be transmitted over the MSP UART TX to the Raspberry Pi.
 */
BaseType_t xUART_to_wire(uint32_t move) {
    return xQueueSend(queue_to_wire, &move, portMAX_DELAY);
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
static void prvUART_UnpackAndSend(UART_Regs* uart, uint32_t packet) {
    uint8_t next_byte;

    for (int i = 0; i < 4; i += 1) {
        next_byte = (uint8_t) (packet & 0xFF);
        DL_UART_transmitDataBlocking(uart, next_byte);
        packet >>= 8;
    }
}

/** 
 * Do the reverse of sending: collect 4 bytes at a time into a single protocol
 * word.
 */
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

static void prvUART_EstablishHeartbeat(void) {
    uint32_t heartbeat_response = 0x00000000;

    do {
        prvUART_UnpackAndSend(RPI_UART_INST, MSP_SYN);
        vTaskDelay(UART_HEARTBEAT_MS / portTICK_PERIOD_MS); // Approximate (low-resolution) 100ms delay 
        // TODO: Check if any messages received. If not, continue to next loop iteration.
    } while (heartbeat_response != RPI_SYNACK);

    prvUART_UnpackAndSend(RPI_UART_INST, MSP_ACK); // Complete the "three-way handshake" and signal to RPi that comms are live
}

/**
 * The main loop for the UART task.
 */
void vUART_Thread(void* arg0) {

    /* Thread-local variables for sending and receiving words */
    uint32_t next_sent;
    uint32_t next_packet;

    while (1) { 
        BaseType_t available_from_main = uxQueueMessagesWaiting(queue_to_wire);

        if (available_from_main > 0) {
            // Send all ready data from main to the Raspberry Pi
            for (BaseType_t i = 0; i < available_from_main; i += 1) {
                next_sent = xQueueReceive(queue_to_wire, &next_sent, portMAX_DELAY);
                prvUART_UnpackAndSend(RPI_UART_INST, next_sent);
            }
        } 

        do {
            next_packet = prvUART_PackAndReceive(RPI_UART_INST);
            xMain_uart_message(next_packet);
        } while (!IS_LAST_MOVE(next_packet)); // While the last packet received is **not** the last packet in this series

        taskYIELD(); // Rather than busy wait on FIFOs, yield to other tasks
    }

}
