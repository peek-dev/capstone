#include "game.h"
#include "assert.h"
#include "config.h"
#include "projdefs.h"

PieceType xGetSquare(BoardState *board, uint8_t row, uint8_t column) {
    assert(row < 8 && column < 8);
    return (PieceType)(0xF & (board->rows[row] >> (4 * column)));
}
void vSetSquare(BoardState *board, uint8_t row, uint8_t column,
                PieceType type) {
    assert(row < 8 && column < 8);
    board->rows[row] &= ~(0xF << (4 * column));
    board->rows[row] |= type << (4 * column);
}

BaseType_t isWhite(PieceType p) { return (p <= WhiteKing) ? pdTRUE : pdFALSE; }

PieceType xChangeColor(PieceType p) {
    // This works only because the bins are symmetric around the middle (empty).
    return BlackPawn - p;
}