/**
 * Copyright (C) <year>  <name of author>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */
#include "util.h"
#include "game.h"

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
