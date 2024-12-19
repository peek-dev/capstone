/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * clock.h: an interface definition to support the chess clock firmware in
 * this project (i.e., that operates the C.H.E.S.S.B.O.A.R.D. LCD clocks).
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
// Only call this if you're REALLY sure you want to explicitly force a rerender.
BaseType_t xClock_render_state(void);

#ifdef DECLARE_PRIVATE_CLOCK_C
enum Clock_MsgType {
    clockmsg_set_time,
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
