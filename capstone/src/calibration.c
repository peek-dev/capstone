#include "config.h"
#include "calibration.h"
#include "assert.h"

static const uint16_t bins[] = {0, 173, 567, 962, 1332, 1708, 1968, 2128, 2387, 2763, 3133, 3528, 3923};

// Lower bounds for hall effect sensor reading bins.
const uint16_t *GetBins(uint8_t row, uint8_t col) {
    assert(row < 8 && col < 8);
    return bins;
  }
