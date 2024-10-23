#include <stdint.h>
#include <stdlib.h> // for ssize_t

#ifndef __MSPM0_RPI5_UART_BIDIR_PROTOCOL_H__
#define __MSPM0_RPI5_UART_BIDIR_PROTOCOL_H__

/**
 * Constants for chessboard files.
 */
typedef uint8_t CHESS_FILE;

#define FILE_A  ((uint8_t) 0)
#define FILE_B  ((uint8_t) 1)
#define FILE_C  ((uint8_t) 2)
#define FILE_D  ((uint8_t) 3)
#define FILE_E  ((uint8_t) 4)
#define FILE_F  ((uint8_t) 5)
#define FILE_G  ((uint8_t) 6)
#define FILE_H  ((uint8_t) 7)

/**
 * Constants for chessboard ranks.
 */
typedef uint8_t CHESS_RANK;

#define RANK_1  ((uint8_t) 0)
#define RANK_2  ((uint8_t) 1)
#define RANK_3  ((uint8_t) 2)
#define RANK_4  ((uint8_t) 3)
#define RANK_5  ((uint8_t) 4)
#define RANK_6  ((uint8_t) 5)
#define RANK_7  ((uint8_t) 6)
#define RANK_8  ((uint8_t) 7)

/**
 * Constants for chess piece types ("ptypes")
 */
typedef uint8_t PTYPE;

#define PTYPE_NULL      ((uint8_t) 0x0)
#define PTYPE_NULL_ALT  ((uint8_t) 0x4)
#define PTYPE_PAWN      ((uint8_t) 0x1)
#define PTYPE_KNIGHT    ((uint8_t) 0x2)
#define PTYPE_BISHOP    ((uint8_t) 0x3)
#define PTYPE_ROOK      ((uint8_t) 0x5)
#define PTYPE_QUEEN     ((uint8_t) 0x6)
#define PTYPE_KING      ((uint8_t) 0x7)

// Friendly ptype aliases
#define PIECE_NULL      PTYPE_NULL
#define PIECE_NULL_ALT  PTYPE_NULL_ALT
#define PAWN            PTYPE_PAWN
#define KNIGHT          PTYPE_KNIGHT
#define BISHOP          PTYPE_BISHOP
#define ROOK            PTYPE_ROOK
#define QUEEN           PTYPE_QUEEN
#define KING            PTYPE_KING

/**
 * Constants for encoded move types ("mtypes")
 */
typedef uint8_t MTYPE;

#define MTYPE_NORMAL            ((uint8_t) 0)
#define MTYPE_CHECK             ((uint8_t) 1)
#define MTYPE_CAPTURE           ((uint8_t) 2)
#define MTYPE_CASTLE_PROMOTE    ((uint8_t) 3)

/**
 * Constants for button events
 */
typedef uint8_t BUTTON_EVENT;

#define BUTTON_START    ((uint8_t) 0)
#define BUTTON_RESTART  ((uint8_t) 1)
#define BUTTON_HINT     ((uint8_t) 2)
#define BUTTON_UNDO     ((uint8_t) 3)

/**
 * Constants (shifts and masks) for upper 16 bits of 32-bit UART encoded word
 * (almost completely shared between Pi->MSP and MSP-Pi packets)
 */
#define SRC_FILE_SHIFT                  29
#define SRC_FILE_MASK                   (((uint32_t) 0x7) << SRC_FILE_SHIFT)
#define GET_SRC_FILE(word)   (uint8_t) ( ((word & SRC_FILE_MASK) >> SRC_FILE_SHIFT) & 0x7 )

#define SRC_RANK_SHIFT                  26
#define SRC_RANK_MASK                   (((uint32_t) 0x7) << SRC_RANK_SHIFT)
#define GET_SRC_RANK(word)   (uint8_t) ( ((word & SRC_RANK_MASK) >> SRC_RANK_SHIFT) & 0x7 )

#define DEST_FILE_SHIFT                 23
#define DEST_FILE_MASK                  (((uint32_t) 0x7) << DEST_FILE_SHIFT)
#define GET_DEST_FILE(word)  (uint8_t) ( ((word & DEST_FILE_MASK) >> DEST_FILE_SHIFT) & 0x7 )

#define DEST_RANK_SHIFT                 20
#define DEST_RANK_MASK                  (((uint32_t) 0x7) << DEST_RANK_SHIFT)
#define GET_DEST_RANK(word)  (uint8_t) ( ((word & DEST_RANK_MASK) >> DEST_RANK_SHIFT) & 0x7 )

#define PTYPE_SHIFT                     17
#define PTYPE_MASK                      (((uint32_t) 0x7) << PTYPE_SHIFT)
#define GET_PTYPE(word)      (uint8_t) ( ((word & PTYPE_MASK) >> PTYPE_SHIFT) & 0x7 )

#define M2_SHIFT                        16
#define M2_MASK                         ((uint32_t) 0x1 << M2_SHIFT)
#define GET_M2(word)         (uint8_t) ( ((word & M2_MASK) >> M2_SHIFT) & 0x1 )

/** 
 * Constants (shifts and masks) for parts of lower 16 bits of 32-bit UART
 * encoded word (shared between Pi->MSP move and undo packets)
 */
#define M2_SRC_FILE_SHIFT       13
#define M2_SRC_FILE_MASK        ((uint32_t) 0x7 << M2_SRC_FILE_SHIFT)
#define GET_M2_SRC_FILE(word)        (uint8_t) ( ((word & M2_SRC_FILE_MASK) >> M2_SRC_FILE_SHIFT) & 0x7)

#define M2_SRC_RANK_SHIFT       10
#define M2_SRC_RANK_MASK        ((uint32_t) 0x7 << M2_SRC_RANK_SHIFT)
#define GET_M2_SRC_RANK(word)        (uint8_t) ( ((word & M2_SRC_RANK_MASK) >> M2_SRC_RANK_SHIFT) & 0x7)

#define M2_DEST_FILE_SHIFT      7
#define M2_DEST_FILE_MASK       ((uint32_t) 0x7 << M2_DEST_FILE_SHIFT)
#define GET_M2_DEST_FILE(word)        (uint8_t) ( ((word & M2_DEST_FILE_MASK) >> M2_DEST_FILE_SHIFT) & 0x7)

#define M2_DEST_RANK_SHIFT      4
#define M2_DEST_RANK_MASK       ((uint32_t) 0x7 << M2_DEST_RANK_SHIFT)
#define GET_M2_DEST_RANK(word)        (uint8_t) ( ((word & M2_DEST_RANK_MASK) >> M2_DEST_RANK_SHIFT) & 0x7)

/** 
 * Constants (shifts and masks) and associated macros for lowest-order 4 bits
 * of 32-bit UART encoded word.
 *
 * The lowest-order 4 bits are variable depending on packet type: Pi->MSP
 * normal move, Pi->MSP undo move, or MSP->Pi record move.
 */
#define MTYPE_SHIFT                             2
#define MTYPE_MASK                              ((uint32_t) 0x3 << MTYPE_SHIFT)
#define GET_MTYPE(word)              (uint8_t) ( ((word & MTYPE_MASK) >> MTYPE_SHIFT) & 0x3 )

#define M2_PTYPE_SHIFT                          1
#define M2_PTYPE_MASK                           ((uint32_t) 0x2)
#define GET_M2_PTYPE(word)           (uint8_t) ( ((word & M2_PTYPE_MASK) >> 1) & 0x1 )

#define IS_LAST_MOVE(word)           ((uint8_t) word & 0x1)

#define CHECK_UNDO_BW(word)          (((uint8_t) word >> 3) & 0x1)

#define GET_UNDO_PTYPE(word)         ((uint8_t) word & 0x7)

#define CHECK_BTN_EVENT(word)        ((uint8_t) word & 0x3)

/**
 * Encoding and decoding interface for MSPM0
 */
typedef struct msp_move_packet {
    CHESS_FILE src_file;
    CHESS_RANK src_rank;
    CHESS_FILE dest_file;
    CHESS_RANK dest_rank;
    PTYPE ptype;    
    MTYPE mtype;
    BUTTON_EVENT button_event;
} msp_packet;

typedef struct rpi_move_packet {
    CHESS_FILE src_file;
    CHESS_RANK src_rank;
    CHESS_FILE dest_file;
    CHESS_RANK dest_rank;
    PTYPE ptype;
    uint8_t m2;
    CHESS_FILE m2_src_file;
    CHESS_RANK m2_src_rank;
    CHESS_FILE m2_dest_file;
    CHESS_RANK m2_dest_rank;
    MTYPE mtype;
    uint8_t m2_ptype;
    uint8_t last;
} rpi_move;

typedef struct rpi_undo_packet {
    CHESS_FILE src_file;
    CHESS_RANK src_rank;
    CHESS_FILE dest_file;
    CHESS_RANK dest_rank;
    PTYPE ptype;
    uint8_t m2;
    CHESS_FILE m2_src_file;
    CHESS_RANK m2_src_rank;
    CHESS_FILE m2_dest_file;
    CHESS_RANK m2_dest_rank;
    uint8_t bw_flag;
    PTYPE undone_ptype;
} rpi_undo;

uint32_t xencode_pickup_for_rpi(CHESS_FILE src_file, CHESS_RANK src_rank, PTYPE ptype);

uint32_t xencode_move_for_rpi(msp_packet* move);

uint32_t xencode_start_for_rpi(void);

uint32_t xencode_restart_for_rpi(void);

uint32_t xencode_hint_for_rpi(CHESS_FILE src_file, CHESS_RANK src_rank);

uint32_t xencode_undo_for_rpi(BUTTON_EVENT undo_event);

void vdecode_move_for_msp(uint32_t word, rpi_move* move);

void vdecode_undo_for_msp(uint32_t word, rpi_undo* undo);

void vsend_packet_common(void* arg, uint32_t word, void (*send_func)(void*, uint32_t));

uint32_t xrecv_packet_common(void* arg, uint32_t(*recv_func)(void*));

/**
 * Encoding and decoding interface for RPi5
 */

uint32_t xencode_move_for_msp(rpi_move* move);

uint32_t xencode_undo_for_msp(rpi_undo* undo);

void vdecode_packet_for_rpi(uint32_t word, msp_packet* packet);

#endif
