#include "uart_bidir_protocol.h"
#include "config.h"
#include "ti/driverlib/dl_uart.h"

/**
 * MSPM0 interface implementation
 */
uint32_t xencode_pickup_for_rpi(CHESS_FILE src_file, CHESS_RANK src_rank,
                                PTYPE ptype) {
    uint32_t encoded_word = 0;
    encoded_word |= (((uint32_t)src_file) << SRC_FILE_SHIFT);
    encoded_word |= (((uint32_t)src_rank) << SRC_RANK_SHIFT);
    encoded_word |= (((uint32_t)ptype) << PTYPE_SHIFT);
    return encoded_word;
}

uint32_t xencode_move_for_rpi(msp_packet *move) {
    uint32_t encoded_word = 0;
    encoded_word |= (((uint32_t)move->src_file) << SRC_FILE_SHIFT);
    encoded_word |= (((uint32_t)move->src_rank) << SRC_RANK_SHIFT);
    encoded_word |= (((uint32_t)move->dest_file) << DEST_FILE_SHIFT);
    encoded_word |= (((uint32_t)move->dest_rank) << DEST_RANK_SHIFT);
    encoded_word |= (((uint32_t)move->ptype) << PTYPE_SHIFT);
    encoded_word |= (((uint32_t)move->mtype) << MTYPE_SHIFT);
    encoded_word |= ((uint32_t)move->button_event);
    return encoded_word;
}

uint32_t xencode_start_for_rpi(void) { return 0 | BUTTON_START; }

uint32_t xencode_restart_for_rpi(void) { return 0 | BUTTON_RESTART; }

uint32_t xencode_hint_for_rpi(CHESS_FILE src_file, CHESS_RANK src_rank) {
    uint32_t encoded_word = 0;
    encoded_word |= (((uint32_t)src_file) << SRC_FILE_SHIFT);
    encoded_word |= (((uint32_t)src_rank) << SRC_RANK_SHIFT);
    encoded_word |= BUTTON_HINT;
    return encoded_word;
}

void vdecode_move_for_msp(uint32_t word, rpi_move *move) {
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

void vdecode_undo_for_msp(uint32_t word, rpi_undo *undo) {
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

