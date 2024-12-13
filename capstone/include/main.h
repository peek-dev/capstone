#ifndef CAPSTONE_MAIN_H
#define CAPSTONE_MAIN_H

#include "game.h"

#define N_STDDEV_BINS 41

typedef enum {
    main_thread_single,
    main_thread_average,
    main_thread_weighted,
    main_thread_requests,
    main_thread_finished
} Main_ThreadMode;

BaseType_t xMain_sensor_update(BoardState *state, uint16_t nreq);
BaseType_t xMain_change_mode(Main_ThreadMode mode, uint16_t stddev_i);
#endif