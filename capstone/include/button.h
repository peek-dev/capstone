#ifndef CAPSTONE_BUTTON_H
#define CAPSTONE_BUTTON_H

#include "config.h"
#include "uart_bidir_protocol.h"

enum button_num {
    button_num_start_restart = BUTTON_START,
    button_num_hint = BUTTON_HINT,
    button_num_undo = BUTTON_UNDO,
    button_num_pause,
    button_num_black_move,
    button_num_white_move,
    button_num_clock_mode,
};

void vButton_Init(void);

#endif