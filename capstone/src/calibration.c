#include "config.h"
#include "calibration.h"
#include "assert.h"

const uint16_t bins[] = {0, 173, 567, 962, 1332, 1708, 1968, 2128, 2387, 2763, 3133, 3528, 3923};
const uint16_t offset_bins[] = {   0,  304,  809, 1167, 1438, 1781, 2004, 2090, 2313, 2656, 2927, 3285, 3790};
const uint16_t offset_bins_triple[] = {    0,   913,  2428,  3502,  4314,  5344,  6013,  6271,  6940,  7970,  8782,  9856,
 11371};
const uint16_t offset_bins_quad[] = {    0,  1217,  3238,  4669,  5753,  7125,  8017,  8362,  9254, 10626, 11710, 13141,
 15162};