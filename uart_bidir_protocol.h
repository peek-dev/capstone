#include <stdint.h>

#ifndef __MSPM0_RPI5_UART_BIDIR_PROTOCOL_H__
#define __MSPM0_RPI5_UART_BIDIR_PROTOCOL_H__

#define BW_FLAG_SHIFT           31
#define SRC_FILE_SHIFT          28
#define SRC_RANK_SHIFT          25
#define SRC_PIECE_SHIFT         22
#define DEST_FILE_SHIFT         19
#define DEST_RANK_SHIFT         16
#define DEST_PIECE_SHIFT        13
#define CHECK_INFO_SHIFT        11

/* Bit shifts for optional flags */
#define PACKET_NUM_SHIFT        6
#define PACKET_TOTAL_SHIFT      1
#define BEST_MOVE_SHIFT         0

typedef uint8_t chess_rank;

/** 
 * A letter-to-int mapping of formal chess file names to short integer data
 * encodings "on the wire".
 *
 * Name clumsy, but chosen to avoid collisions with "file"
 */
typedef enum chess_file_enum { A=0, B, C, D, E, F, G, H } chess_file;

/**
 * A piece type-to-int mapping of chess pieces (by named type) to short 
 * integer data encodings "on the wire".
 */
typedef enum chess_piece_enum {
    NONE=0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK=5, /* skip from 3 to 5 to allow encoding 4 as alias for None */
    QUEEN,
    KING
} chess_piece;

typedef enum chess_move_check_info {
    NONE=0,
    CHECK_CLEARED,
    CHECK,
    CHECKMATE
} move_check_info;

typedef struct chess_uart_message {
    /* Whether move was made by black or white */
    uint8_t bw_flag;

    /* States to encode source and piece moved */
    chess_file src_file;
    chess_rank src_rank;
    chess_piece src_piece;

    /* States to encode destination and opposing piece captured (as applicable) */
    chess_file dest_file;
    chess_rank dest_rank;
    chess_piece dest_piece;

    /* Information about whether the move creates check, resolves check, creates checkmate, or none of the above. */
    move_check_info check_info;

    /* Optional fields to implement sequential packet transmission and reception as well as "best move" designation */
    uint8_t packet_no;
    uint8_t packet_total;
    uint8_t best_move;
} chess_message;

chess_message* xdecode_for_rpi5(uint32_t bitstring);

void vdecode_for_mspm0(uint32_t bitstring, chess_message* decoded_message);

uint32_t xencode_move(chess_message* message);

#endif
