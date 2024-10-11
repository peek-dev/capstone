#include "globals.h"
#include "game.h"
#include "assert.h"

PieceType xGetSquare(BoardState *board, uint8_t row, uint8_t column) {
    assert(row < 8 && column < 8);
    return (PieceType) (0xF (board[row] >> (4*column)));
}
void vSetSquare(BoardState *board, uint8_t row, uint8_t column, PieceType type) {
    assert(row < 8 && column < 8);
    board[row] &= ~(0xF << (4*column));
    board[row] |=  type << (4*column);
}