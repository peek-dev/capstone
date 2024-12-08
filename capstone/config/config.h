#ifndef CAPSTONE_CONFIG_H
#define CAPSTONE_CONFIG_H

// For the moment, allow assertions despite the code size and memory overhead.
#undef NDEBUG
#define MAKEVISIBLE volatile
#include <inttypes.h>
#include "ti_msp_dl_config.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portmacro.h"

#define IS_MSP

#endif