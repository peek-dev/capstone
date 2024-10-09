#include "uart_bidir_protocol.h"
#include <stdlib.h>

uint32_t xencode_move(chess_message* message) {
    uint32_t constructed_message = 0;
    constructed_message |= (message->bw_flag << BW_FLAGSHIFT);
    constructed_message |= (message->src_file << SRC_FILE_SHIFT);
    constructed_message |= (message->src_rank << SRC_RANK_SHIFT); 
    constructed_message |= (message->src_piece << SRC_PIECE_SHIFT);
    constructed_message |= (message->dest_file << DEST_FILE_SHIFT);
    constructed_message |= (message->dest_rank << DEST_RANK_SHIFT);
    constructed_message |= ((message->dest_piece) & DEST_PIECE_MASK) << DEST_PIECE_SHIFT;
    constructed_message |= ((message->check_info) & CHECK_INFO_MASK) << CHECK_INFO_SHIFT; 

    // Shift in optional flags
    constructed_message |= ((message->
}

