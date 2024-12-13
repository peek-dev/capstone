#include "util.h"
#include "game.h"
#include <stdlib.h>

// Barely modified from https://stackoverflow.com/a/2117018
uint16_t MedianOfFive(uint16_t *arr) {
    uint16_t a, b, c, d, e;
    a = arr[0];
    b = arr[1];
    c = arr[2];
    d = arr[3];
    e = arr[4];
    return b < a   ? d < c   ? b < d   ? a < e   ? a < d   ? e < d ? e : d
                                                   : c < a ? c
                                                           : a
                                         : e < d ? a < d ? a : d
                                         : c < e ? c
                                                 : e
                               : c < e ? b < c   ? a < c ? a : c
                                         : e < b ? e
                                                 : b
                               : b < e ? a < e ? a : e
                               : c < b ? c
                                       : b
                     : b < c ? a < e   ? a < c   ? e < c ? e : c
                                         : d < a ? d
                                                 : a
                               : e < c ? a < c ? a : c
                               : d < e ? d
                                       : e
                     : d < e ? b < d   ? a < d ? a : d
                               : e < b ? e
                                       : b
                     : b < e ? a < e ? a : e
                     : d < b ? d
                             : b
           : d < c ? a < d   ? b < e   ? b < d   ? e < d ? e : d
                                         : c < b ? c
                                                 : b
                               : e < d ? b < d ? b : d
                               : c < e ? c
                                       : e
                     : c < e ? a < c   ? b < c ? b : c
                               : e < a ? e
                                       : a
                     : a < e ? b < e ? b : e
                     : c < a ? c
                             : a
           : a < c ? b < e   ? b < c   ? e < c ? e : c
                               : d < b ? d
                                       : b
                     : e < c ? b < c ? b : c
                     : d < e ? d
                             : e
           : d < e ? a < d   ? b < d ? b : d
                     : e < a ? e
                             : a
           : a < e ? b < e ? b : e
           : d < a ? d
                   : a;
}

uint32_t sum(uint16_t *arr, uint8_t len) {
    uint32_t accumulator = 0;
    for (uint8_t i = 0; i < len; i++) {
        accumulator += arr[i];
    }
    return accumulator;
}

uint16_t average(uint16_t *arr, uint8_t len) { return sum(arr, len) / len; }

uint16_t outlier(uint16_t *arr, uint8_t len) {
    uint16_t avg = average(arr, len);
    int16_t max_diff = 0;
    uint16_t max_ind = 0;
    for (uint8_t i = 0; i < len; i++) {
        int16_t diff = abs(((int16_t)avg) - ((int16_t)arr[i]));
        if (diff > max_diff) {
            max_diff = diff;
            max_ind = i;
        }
    }
    return arr[max_ind];
}
