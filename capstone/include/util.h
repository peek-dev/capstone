#ifndef CAPSTONE_UTIL_H
#define CAPSTONE_UTIL_H
#include "game.h"

uint16_t MedianOfFive(uint16_t *arr);
uint16_t average(uint16_t *arr, uint8_t len);
uint32_t sum(uint16_t *arr, uint8_t len);
uint16_t outlier(uint16_t *arr, uint8_t len);
#endif