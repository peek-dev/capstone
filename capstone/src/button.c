#include "config.h"
#include "ti_msp_dl_config.h"
#include "main.h"

void GPIOA_Handler(void) {
    switch (BUTTON_GPIO_GPIOA_INT_IIDX) {
        case BUTTON_GPIO_PIN_HINT_IIDX:
            xMain_button_press(button_num_hint);
            break;
        case BUTTON_GPIO_PIN_START_RESTART_IIDX:
            xMain_button_press(button_num_start_restart);
        case BUTTON_GPIO_PIN_CLOCK_MODE_IIDX:
            xMain_button_press(button_num_clock_mode);
            break;
        case BUTTON_GPIO_PIN_BLACK_MOVE_IIDX:
            xMain_button_press(button_num_black_move);
        case BUTTON_GPIO_PIN_WHITE_MOVE_IIDX:
            xMain_button_press(button_num_white_move);
            break;
        default:
            break;
    }
}

void GPIOB_Handler(void) {
    switch (BUTTON_GPIO_GPIOB_INT_IIDX) {
        case BUTTON_GPIO_PIN_UNDO_IIDX:
            xMain_button_press(button_num_undo);
            break;
        case BUTTON_GPIO_PIN_PAUSE_IIDX:
            xMain_button_press(button_num_pause);
            break;
    }
}
