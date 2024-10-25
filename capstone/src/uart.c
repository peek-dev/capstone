#include "uart.h"
#include "game.h"
#include "projdefs.h"

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