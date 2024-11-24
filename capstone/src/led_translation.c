#include "config.h"
#include "led_translation.h"
#include "game.h"
#include "assert.h"

ZeroToTwoInts LEDTrans_Ptype(PieceType p) {
    ZeroToTwoInts z;
    z.len = 2;
    z.data[0] = 0;
    z.data[1] = 7;
    switch (p) {
    case EmptySquare:
        z.len -= 1;
    case BlackKing:
        z.data[0] += 1;
    case BlackQueen:
        z.data[0] += 1;
        z.len -= 1;
    case BlackBishop:
        // Shift one in.
        z.data[0] += 1;
        z.data[1] -= 1;
    case BlackKnight:
        // Shift one in.
        z.data[0] += 1;
        z.data[1] -= 1;
    case BlackRook:
        // Shift up one row and then one.
        z.data[0] += 1 * 8 + 1;
        // The other one is still down at WhiteBishop.
        // Two to move to rook, nine rows, two pawns.
        z.data[1] += 2 + 9 * 8 + 2;
        z.len += 1;
    case BlackPawn:
        // Shift up by eight rows, and then one (one after)
        z.data[0] += 5 * 8 + 1;
    case WhitePawn:
        // Skip the rest of the piece row (3), skip two rows, and shift one.
        z.data[0] += 3 + 2 * 8 + 1;
    case WhiteQueen:
        // Shift one up.
        z.data[0] += 1;
    case WhiteKing:
        // Shift one up, and decrease the length.
        z.data[0] += 1;
        z.len -= 1;
    case WhiteBishop:
        // Shift one in.
        z.data[0] += 1;
        z.data[1] -= 1;
    case WhiteKnight:
        // Shift one in.
        z.data[0] += 1;
        z.data[1] -= 1;
    case WhiteRook:
        // Initial state. Change nothing.
        break;
    }
    return z;
}

uint8_t LEDTrans_Square(uint8_t row, uint8_t col) {
    assert(row < 8 && col < 8);
    // Skip the first row of pieces, a pawn if row>=3, and another if row>=8
    uint8_t index = 8 + (row >= 2 ? 1 : 0) + (row == 7 ? 1 : 0);
    // Offset by the number of rows.
    index += 8 * row;
    // Even rows go forward, odd rows go backward.
    index += (row % 2 == 0) ? col : (7 - col);
    return index;
}