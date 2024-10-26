#include "chess.h"
#include "config.h"
#include "game.h"
#include "uart.h"
#include "uart_bidir_protocol.h"

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
