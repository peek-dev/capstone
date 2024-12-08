#ifndef CAPSTONE_CLOCK_H
#define CAPSTONE_CLOCK_H

#include "config.h"
#include "portmacro.h"
#include "game.h"

BaseType_t xClock_Init(void);
void vClock_Thread(void *arg0);

typedef enum {
    clock_state_running,
    clock_state_paused,
    clock_state_off,
    // TODO make undo rendering work
    clock_state_undo,
    clock_state_notstarted,
    clock_state_staticnumbers
} clock_state;

BaseType_t xClock_run_test(uint8_t seconds_per_test);
BaseType_t xClock_set_turn(game_turn turn);
BaseType_t xClock_set_state(clock_state state);
BaseType_t xClock_set_increment(uint32_t increment_ms);
BaseType_t xClock_set_times(uint32_t *times_ms);
BaseType_t xClock_set_numbers(uint16_t *numbers);
BaseType_t xClock_set_both_numbers(uint16_t number);

#ifdef DELCLARE_PRIVATE_CLOCK_C
enum Clock_MsgType {
    clockmsg_set_time,
    // TODO increment
    clockmsg_set_increment,
    clockmsg_render_state,
    clockmsg_set_state,
    clockmsg_set_turn,
    clockmsg_test_segments,
    clockmsg_set_numbers
};

typedef struct {
    enum Clock_MsgType type;
    union {
        uint32_t times[2];
        uint8_t seconds_per_test;
        game_turn turn;
        clock_state state;
        uint16_t numbers[2];
    };
} Clock_Message;
#endif
#endif
