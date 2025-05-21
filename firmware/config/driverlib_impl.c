#include "config.h"
#if HARDWARE_REVISION == 1
#include "v1/ti_msp_dl_config.c"
#elif HARDWARE_REVISION == 2
#include "v2/ti_msp_dl_config.c"
#endif