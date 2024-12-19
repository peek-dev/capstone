#ifndef CAPSTONE_LED_TRANSLATION_H
#define CAPSTONE_LED_TRANSLATION_H

#include "game.h"

typedef struct {
    uint8_t len;
    uint8_t data[2];
} ZeroToTwoInts;

ZeroToTwoInts LEDTrans_Ptype(PieceType p);
uint8_t LEDTrans_Square(uint8_t row, uint8_t col);
#endif