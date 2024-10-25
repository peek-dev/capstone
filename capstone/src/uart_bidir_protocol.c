#include "uart_bidir_protocol.h"

#ifdef IS_MSP
// TODO: include TI driverlib "dl_uart.h"
#endif

#ifdef IS_RPI
#include <stdio.h>
#endif

extern void DL_UART_transmitDataBlocking(UART_Regs* uart, uint8_t data);

extern uint8_t DL_UART_receiveDataBlocking(UART_Regs* uart);

/**
 * Wrapper functions for MSP UART transmit and receive
 */
static void vMSP_UART_xmit_wrapper(void* uart_ptr, uint32_t word) {
    UART_Regs* regs = (UART_Regs*) uart_ptr;
    uint8_t next_word = 0;

    for (int i = 0; i < 4; i += 1) {
        next_word = (uint8_t) ((word >> (i*8)) & 0xFF);
        DL_UART_transmitDataBlocking(regs, word);
    }
}

static uint32_t xMSP_UART_recv_wrapper(void* uart_ptr) {
    UART_Regs* regs = (UART_Regs*) uart_ptr;
    uint32_t full_word = 0;
    uint8_t next_word = 0;

    for (int i = 0; i < 4; i += 1) {
        next_word = DL_UART_receiveDataBlocking(regs);
        full_word |= (((uint32_t) next_word) & 0xFF) << (i * 8);
    }

    return full_word;
}

static void vRPI5_UART_xmit_wrapper(void* uart_fileptr, uint32_t word) {
    FILE* uart_filestream = (FILE*) uart_fileptr;
    uint8_t next_word = 0;

    for (int i = 0; i < 4; i += 1) {
        next_word = (uint8_t) ((word >> (i*8)) & 0xFF);
        fwrite(&next_word, sizeof(char), 1, uart_filestream);
    }
}

static uint32_t vRPI5_UART_recv_wrapper(void* uart_fileptr) {
    FILE* uart_filestream = (FILE*) uart_fileptr;
    uint32_t full_word = 0;
    uint8_t next_word = 0;

    for (int i = 0; i < 4; i += 1) {
        fread(&next_word, sizeof(char), 1, uart_filestream);
        full_word |= (((uint32_t) next_word) & 0xFF) << (i * 8);
    }

    return full_word;
}

/**
 * MSPM0 interface implementation
 */
uint32_t xencode_pickup_for_rpi(CHESS_FILE src_file, CHESS_RANK src_rank, PTYPE ptype) {
    uint32_t encoded_word = 0;
    encoded_word |= ( ((uint32_t) src_file) << SRC_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) src_rank) << SRC_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) ptype) << PTYPE_SHIFT );
    return encoded_word;
}

uint32_t xencode_move_for_rpi(msp_packet* move) {
    uint32_t encoded_word = 0;
    encoded_word |= ( ((uint32_t) move->src_file) << SRC_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) move->src_rank) << SRC_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) move->dest_file) << DEST_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) move->dest_rank) << DEST_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) move->ptype) << PTYPE_SHIFT );
    encoded_word |= ( ((uint32_t) move->mtype) << MTYPE_SHIFT );
    encoded_word |= ( (uint32_t) move->button_event );
    return encoded_word;
}

uint32_t xencode_start_for_rpi(void) {
    return 0 | BUTTON_START;
}

uint32_t xencode_restart_for_rpi(void) {
    return 0 | BUTTON_RESTART;
}

uint32_t xencode_hint_for_rpi(CHESS_FILE src_file, CHESS_RANK src_rank) {
    uint32_t encoded_word = 0;
    encoded_word |= ( ((uint32_t) src_file) << SRC_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) src_rank) << SRC_RANK_SHIFT );
    encoded_word |= BUTTON_HINT;
    return encoded_word;
}

void vdecode_move_for_msp(uint32_t word, rpi_move* move) {
    move->src_file = GET_SRC_FILE(word);
    move->src_rank = GET_SRC_RANK(word);
    move->dest_file = GET_DEST_FILE(word);
    move->dest_rank = GET_DEST_RANK(word);
    move->ptype = GET_PTYPE(word);
    move->m2 = GET_M2(word);
    move->m2_src_file = GET_M2_SRC_FILE(word);
    move->m2_src_rank = GET_M2_SRC_RANK(word);
    move->m2_dest_file = GET_M2_DEST_FILE(word);
    move->m2_dest_rank = GET_M2_DEST_RANK(word);
    move->mtype = GET_MTYPE(word);
    move->m2_ptype = (word & 0x2) >> 1;
    move->last = word & 0x1;
}

void vdecode_undo_for_msp(uint32_t word, rpi_undo* undo) {
    undo->src_file = GET_SRC_FILE(word);
    undo->src_rank = GET_SRC_RANK(word);
    undo->dest_file = GET_DEST_FILE(word);
    undo->dest_rank = GET_DEST_RANK(word);
    undo->ptype = GET_PTYPE(word);
    undo->m2 = GET_M2(word);
    undo->m2_src_file = GET_M2_SRC_FILE(word);
    undo->m2_src_rank = GET_M2_SRC_RANK(word);
    undo->m2_dest_file = GET_M2_DEST_FILE(word);
    undo->m2_dest_rank = GET_M2_DEST_RANK(word);
    undo->bw_flag = CHECK_UNDO_BW(word);
    undo->undone_ptype = GET_UNDO_PTYPE(word);
}

void vsend_packet_common(void* arg, uint32_t word, (void)(*send_func)(void*, uint32_t)) {
    send_func(arg, word);
}

uint32_t xrecv_packet_common(void* arg, uint32_t (*recv_func)(void*)) {
    return recv_func(arg);
}

/**
 * RPi5 interface implementation
 */
uint32_t xencode_move_for_msp(rpi_move* move) {
    uint32_t encoded_word = 0;
    encoded_word |= ( ((uint32_t) move->src_file) << SRC_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) move->src_rank) << SRC_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) move->dest_file) << DEST_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) move->dest_rank) << DEST_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) move->ptype) << PTYPE_SHIFT );
    encoded_word |= ( ((uint32_t) move->m2) << M2_SHIFT );
    encoded_word |= ( ((uint32_t) move->m2_src_file) << M2_SRC_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) move->m2_src_rank) << M2_SRC_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) move->m2_dest_file) << M2_DEST_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) move->m2_dest_rank) << M2_DEST_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) move->mtype) << MTYPE_SHIFT );
    encoded_word |= ( ((uint32_t) move->m2_ptype) << M2_PTYPE_SHIFT );
    encoded_word |= move->last;
    return encoded_word;
}

uint32_t xencode_undo_for_msp(rpi_undo* undo) {
    uint32_t encoded_word = 0;
    encoded_word |= ( ((uint32_t) undo->src_file) << SRC_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) undo->src_rank) << SRC_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) undo->dest_file) << DEST_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) undo->dest_rank) << DEST_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) undo->ptype) << PTYPE_SHIFT );
    encoded_word |= ( ((uint32_t) undo->m2) << M2_SHIFT );
    encoded_word |= ( ((uint32_t) undo->m2_src_file) << M2_SRC_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) undo->m2_src_rank) << M2_SRC_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) undo->m2_dest_file) << M2_DEST_FILE_SHIFT );
    encoded_word |= ( ((uint32_t) undo->m2_dest_rank) << M2_DEST_RANK_SHIFT );
    encoded_word |= ( ((uint32_t) undo->bw_flag) << 0x3 );
    encoded_word |= ((uint32_t) undo->undone_ptype);
    return encoded_word;
}

void vdecode_packet_for_rpi(uint32_t word, msp_packet* packet) {
    packet->src_file = GET_SRC_FILE(word);
    packet->src_rank = GET_SRC_RANK(word);
    packet->dest_file = GET_DEST_FILE(word);
    packet->dest_rank = GET_DEST_RANK(word);
    packet->ptype = GET_PTYPE(word);
    packet->mtype = GET_MTYPE(word);
    packet->button_event = CHECK_BTN_EVENT(word);
}

