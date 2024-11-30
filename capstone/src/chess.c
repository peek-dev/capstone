#include "chess.h"
#include "config.h"
#include "game.h"
#include "led.h"
#include "portmacro.h"
#include "projdefs.h"
#include "uart.h"
#include "uart_bidir_protocol.h"
#include "led_translation.h"

BaseType_t xCheckValidMove(BoardState *old, BoardState *new, NormalMove move) {
    BaseType_t whiteToMove =
        isWhite(xGetSquare(old, GET_SRC_RANK(move), GET_SRC_FILE(move)));
    // Technically, this is a nasty shortcut. Beware, if the uart protocol
    // changes, these will need to too.
    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t col = 0; col < 8; col++) {
            PieceType post = xGetSquare(new, row, col);
            PieceType expected;
            // If this square should change, confirm that it does.
            if (row == GET_SRC_RANK(move) && col == GET_SRC_FILE(move)) {
                // Squares that pieces moved from should be empty
                // TODO validate this with complex rules e.g. castling, promo.
                expected = EmptySquare;
            } else if (row == GET_DEST_RANK(move) &&
                       col == GET_DEST_FILE(move)) {
                expected = xPtypeFromWire(GET_PTYPE(move), whiteToMove);
            } else if (row == GET_M2_DEST_RANK(move) &&
                       col == GET_M2_DEST_FILE(move)) {
                // For M2, we check dest first. This is because having dest and
                // src be the same square is valid for a second move (e.g. en
                // passant). It's okay to use whiteToMove to determine color,
                // since the only second move involving another player's piece
                // is en passant, which gives EmptySquare for either.
                expected = xPtypeFromWire(
                    GET_M2_PTYPE(move) ? PTYPE_ROOK : PTYPE_NULL, whiteToMove);
            } else if (row == GET_M2_SRC_RANK(move) &&
                       col == GET_M2_SRC_FILE(move)) {
                // If this square was a source but not a destination, it should
                // be empty.
                expected = EmptySquare;
            } else {
                // If this was not touched by the move, it should be the same as
                // before.
                expected = xGetSquare(old, row, col);
            }

            // Confirm our expectations.
            if (post != expected) {
                return pdFALSE;
            }
        }
    }
    return pdTRUE;
}

int16_t sFindMoveIndex(BoardState *old, BoardState *new, NormalMove *moves,
                       uint16_t moves_len) {
    // Search every available move.
    for (uint16_t i = 0; i < moves_len; i++) {
        // If one matches, return the index.
        if (xCheckValidMove(old, new, moves[i]) == pdTRUE) {
            return i;
        }
    }
    // No matches
    return -1;
}

// This function is intended to find whether exactly one piece has been lifted
// (and not returned) from the board. row and column are outputs.
// TODO maybe make this hybrid with findvalidmove, and include return states for
// needing a second-move completion?
BaseType_t xFindSingleLifted(BoardState *old, BoardState *new,
                             uint8_t *found_row, uint8_t *found_col) {
    BaseType_t found = pdFALSE;
    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t col = 0; col < 8; col++) {
            if (xGetSquare(old, row, col) != xGetSquare(new, row, col)) {
                if (xGetSquare(new, row, col) == EmptySquare) {
                    // If we haven't already found a missing piece..
                    if (found == pdFALSE) {
                        *found_row = row;
                        *found_col = col;
                        found = pdTRUE;
                    } else {
                        // Multiple pieces have been removed.
                        return pdFALSE;
                    }
                } else {
                    // Some other change occured.
                    return pdFALSE;
                }
            }
        }
    }
    return found;
}

BaseType_t xIlluminateMove(NormalMove move, uint8_t do_src) {
    Color color;
    uint8_t row;
    uint8_t col;
    if (do_src != 0) {
        row = GET_SRC_RANK(move);
        col = GET_SRC_FILE(move);
        color.red = 255;
        color.green = 255;
        color.blue = 255;
        color.brightness = 10;
    } else {
        color.brightness = 31;
        row = GET_DEST_RANK(move);
        col = GET_DEST_FILE(move);
        color.red = 0;
        color.green = 0;
        color.blue = 0;
        switch (GET_MTYPE(move)) {
        case MTYPE_NORMAL:
            color.blue = 255;
        case MTYPE_CHECK:
            color.green = 255;
        case MTYPE_CAPTURE:
            color.red = 255;
            break;
        case MTYPE_CASTLE_PROMOTE:
            color.red = 150;
            color.green = 0;
            color.blue = 255;
            break;
        }
    }
    return xLED_set_color(LEDTrans_Square(row, col), &color);
}

BaseType_t xIlluminateMovable(NormalMove *moves, uint16_t moves_len) {
    for (uint16_t i = 0; i < moves_len; i++) {
        // Only render if it's a new source square. This assumes the moves are sorted
        // by source square, which should be true. Worst-case, this re-renders some squares.
        if (i == 0 || GET_SRC_RANK(moves[i]) != GET_SRC_RANK(moves[i-1]) || GET_SRC_FILE(moves[i]) != GET_SRC_FILE(moves[i-1])) {
            if (xIlluminateMove(moves[i], 1) != pdPASS) return pdFAIL;
        }
    }
    return pdPASS;
}

// TODO optimize interleave this with findsinglelifted?
BaseType_t xIlluminatePieceMoves(NormalMove *moves, uint16_t moves_len, uint8_t row, uint8_t col) {
    for (uint16_t i = 0; i < moves_len; i++) {
        if (GET_SRC_RANK(moves[i]) == row && GET_SRC_FILE(moves[i]) == col) {
            xIlluminateMove(moves[i], 0);
        }
    }
}