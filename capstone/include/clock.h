#ifndef CAPSTONE_CLOCK_H
#define CAPSTONE_CLOCK_H

#include "config.h"

BaseType_t xClock_Init(void);
void vClock_Thread(void *arg0);

BaseType_t xClock_run_test(uint8_t seconds_per_test);
BaseType_t xClock_set_turn(uint8_t is_black);
BaseType_t xClock_stop_clock(void);
BaseType_t xClock_start_clock(void);
BaseType_t xClock_set_times(uint32_t *times_ms);

#ifdef DELCLARE_PRIVATE_CLOCK_C
enum Clock_MsgType {
    clockmsg_set_time,
    clockmsg_writehw,
    clockmsg_stop,
    clockmsg_start,
    clockmsg_set_turn,
    clockmsg_test_segments,
};

typedef struct {
    enum Clock_MsgType type;
    union {
        uint32_t times[2];
        uint8_t seconds_per_test;
        uint8_t set_turn_black;
    };
} Clock_Message;
#endif
#endif
