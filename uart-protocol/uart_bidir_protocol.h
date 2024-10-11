#include <stdint.h>

#ifndef __MSPM0_RPI5_UART_BIDIR_PROTOCOL_H__
#define __MSPM0_RPI5_UART_BIDIR_PROTOCOL_H__

/**
 * Constants for chessboard files.
 */
#define FILE_A  ((uint32_t) 0)
#define FILE_B  ((uint32_t) 1)
#define FILE_C  ((uint32_t) 2)
#define FILE_D  ((uint32_t) 3)
#define FILE_E  ((uint32_t) 4)
#define FILE_F  ((uint32_t) 5)
#define FILE_G  ((uint32_t) 6)
#define FILE_H  ((uint32_t) 7)

/**
 * Constants for chessboard ranks.
 */
#define RANK_1  ((uint32_t) 0)
#define RANK_2  ((uint32_t) 1)
#define RANK_3  ((uint32_t) 2)
#define RANK_4  ((uint32_t) 3)
#define RANK_5  ((uint32_t) 4)
#define RANK_6  ((uint32_t) 5)
#define RANK_7  ((uint32_t) 6)
#define RANK_8  ((uint32_t) 7)

/**
 * Constants for chess piece types ("ptypes")
 */
#define PTYPE_NULL      ((uint32_t) 0x0)
#define PTYPE_NULL_ALT  ((uint32_t) 0x4)
#define PTYPE_PAWN      ((uint32_t) 0x1)
#define PTYPE_KNIGHT    ((uint32_t) 0x2)
#define PTYPE_BISHOP    ((uint32_t) 0x3)
#define PTYPE_ROOK      ((uint32_t) 0x5)
#define PTYPE_QUEEN     ((uint32_t) 0x6)
#define PTYPE_KING      ((uint32_t) 0x7)

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
#define MTYPE_NORMAL            ((uint32_t) 0)
#define MTYPE_CHECK             ((uint32_t) 1)
#define MTYPE_CAPTURE           ((uint32_t) 2)
#define MTYPE_CASTLE_PROMOTE    ((uint32_t) 3)

/**
 * Constants for button events
 */
#define BUTTON_START    ((uint32_t) 0)
#define BUTTON_RESTART  ((uint32_t) 1)
#define BUTTON_HINT     ((uint32_t) 2)
#define BUTTON_UNDO     ((uint32_t) 3)

/**
 * Constants (shifts and masks) for upper 16 bits of 32-bit UART encoded word
 * (almost completely shared between Pi->MSP and MSP-Pi packets)
 */
#define SRC_FILE_SHIFT                  29
#define SRC_FILE_MASK                   (((uint32_t) 0x7) << SRC_FILE_SHIFT)
#define GET_SRC_FILE((uint32_t) word)   (uint32_t) ( ((word & SRC_FILE_MASK) >> SRC_FILE_SHIFT) & 0x7 )

#define SRC_RANK_SHIFT                  26
#define SRC_RANK_MASK                   (((uint32_t) 0x7) << SRC_RANK_SHIFT)
#define GET_SRC_RANK((uint32_t) word)   (uint32_t) ( ((word & SRC_RANK_MASK) >> SRC_RANK_SHIFT) & 0x7 )

#define DEST_FILE_SHIFT                 23
#define DEST_FILE_MASK                  (((uint32_t) 0x7) << DEST_FILE_SHIFT)
#define GET_DEST_FILE((uint32_t) word)  (uint32_t) ( ((word & DEST_FILE_MASK) >> DEST_FILE_SHIFT) & 0x7 )

#define DEST_RANK_SHIFT                 20
#define DEST_RANK_MASK                  (((uint32_t) 0x7) << DEST_RANK_SHIFT)
#define GET_DEST_RANK((uint32_t) word)  (uint32_t) ( ((word & DEST_RANK_MASK) >> DEST_RANK_SHIFT) & 0x7 )

#define PTYPE_SHIFT                     17
#define PTYPE_MASK                      (((uint32_t) 0x7) << PTYPE_SHIFT)
#define GET_PTYPE((uint32_t) word)      (uint32_t) ( ((word & PTYPE_MASK) >> PTYPE_SHIFT) & 0x7 )

#define M2_SHIFT                        16
#define M2_MASK                         ((uint32_t) 0x1 << M2_SHIFT)
#define GET_M2((uint32_t) word)         (uint32_t) ( ((word & M2_MASK) >> M2_SHIFT) & 0x1 )

/** 
 * Constants (shifts and masks) for parts of lower 16 bits of 32-bit UART
 * encoded word (shared between Pi->MSP move and undo packets)
 */
#define M2_SRC_FILE_SHIFT       13
#define M2_SRC_FILE_MASK        ((uint32_t) 0x7 << M2_SRC_FILE_SHIFT)
#define GET_M2_SRC_FILE((uint32_t) word)        (uint32_t) ( ((word & M2_SRC_FILE_MASK) >> M2_SRC_FILE_SHIFT) & 0x7)

#define M2_SRC_RANK_SHIFT       10
#define M2_SRC_RANK_MASK        ((uint32_t) 0x7 << M2_SRC_RANK_SHIFT)
#define GET_M2_SRC_RANK((uint32_t) word)        (uint32_t) ( ((word & M2_SRC_RANK_MASK) >> M2_SRC_RANK_SHIFT) & 0x7)

#define M2_DEST_FILE_SHIFT      7
#define M2_DEST_FILE_MASK       ((uint32_t) 0x7 << M2_DEST_FILE_SHIFT)
#define GET_M2_DEST_FILE((uint32_t) word)        (uint32_t) ( ((word & M2_DEST_FILE_MASK) >> M2_DEST_FILE_SHIFT) & 0x7)

#define M2_DEST_RANK_SHIFT      4
#define M2_DEST_RANK_MASK       ((uint32_t) 0x7 << M2_DEST_RANK_SHIFT)
#define GET_M2_DEST_RANK((uint32_t) word)        (uint32_t) ( ((word & M2_DEST_RANK_MASK) >> M2_DEST_RANK_SHIFT) & 0x7)

/** 
 * Constants (shifts and masks) and associated macros for lowest-order 4 bits
 * of 32-bit UART encoded word.
 *
 * The lowest-order 4 bits are variable depending on packet type: Pi->MSP
 * normal move, Pi->MSP undo move, or MSP->Pi record move.
 */
#define MTYPE_SHIFT                             2
#define MTYPE_MASK                              ((uint32_t) 0x3 << MTYPE_SHIFT)
#define GET_MTYPE((uint32_t) word)              (uint32_t) ( ((word & MTYPE_MASK) >> MTYPE_SHIFT) & 0x3 )

#define M2_PTYPE_SHIFT                          1
#define M2_PTYPE_MASK                           ((uint32_t) 0x2)
#define GET_M2_PTYPE((uint32_t) word)           (uint32_t) ( ((word & M2_PTYPE_MASK) >> 1) & 0x1 )

#define IS_LAST_MOVE((uint32_t) word)           ((uint32_t) word & 0x1)

#define CHECK_UNDO_BW((uint32_t) word)          (((uint32_t) word >> 3) & 0x1)

#define GET_UNDO_PTYPE((uint32_t) word)         ((uint32_t) word & 0x7)

#define CHECK_BTN_EVENT((uint32_t) word)        ((uint32_t) word & 0x3)

/**
 * Encoding and decoding interface for MSPM0
 */
typedef struct msp_move_packet {
    uint32_t src_file;
    uint32_t src_rank;
    uint32_t dest_file;
    uint32_t dest_rank;
    uint32_t ptype;    
    uint32_t mtype;
    uint32_t button_event;
} msp_packet;

uint32_t xencode_pickup_for_rpi(uint32_t src_file, uint32_t src_rank, uint32_t ptype);

uint32_t xencode_move_for_rpi(msp_packet* validated_move_packet);

uint32_t xencode_start_for_rpi(void);
uint32_t xencode_restart_for_rpi(void);

uint32_t xencode_hint_for_rpi(uint32_t src_file, uint32_t src_rank, uint32_t hint_event);

uint32_t xencode_undo_for_rpi(uint32_t undo_event);

/**
 * Encoding and decoding interface for RPi5
 * TODO: add more
 */

#endif
