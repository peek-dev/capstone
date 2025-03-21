/**
 * Copyright (C) 2024 Paul D. Karhnak
 *
 * uart_bidir_protocol.c: the implementation source file for the custom UART
 * protocol coinvented by Paul D. Karhnak and John E. Berberian, Jr., to
 * support the C.H.E.S.S.B.O.A.R.D. communication between its MSPM0G3507 and
 * Raspberry Pi computer systems.
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

uint32_t xencode_restart_for_rpi(void) { return 0 | BUTTON_RESTART; }

uint32_t xencode_undo_for_rpi(void) { return 0 | BUTTON_UNDO; }

uint32_t xencode_hint_for_rpi(CHESS_FILE src_file, CHESS_RANK src_rank) {
    uint32_t encoded_word = 0;
    encoded_word |= (((uint32_t)src_file) << SRC_FILE_SHIFT);
    encoded_word |= (((uint32_t)src_rank) << SRC_RANK_SHIFT);
    encoded_word |= BUTTON_HINT;
    return encoded_word;
}
