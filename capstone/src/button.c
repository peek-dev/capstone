#include "button.h"
#include "config.h"
#include "ti/driverlib/dl_gpio.h"
#include "ti_msp_dl_config.h"
#include "main.h"

static const uint32_t LenA = 5;
static const uint32_t PinsA[] = {
    BUTTON_GPIO_PIN_HINT_PIN, BUTTON_GPIO_PIN_START_RESTART_PIN,
    BUTTON_GPIO_PIN_CLOCK_MODE_PIN, BUTTON_GPIO_PIN_BLACK_MOVE_PIN,
    BUTTON_GPIO_PIN_WHITE_MOVE_PIN};
static const enum button_num ButtonsA[] = {
    button_num_hint, button_num_start_restart, button_num_clock_mode,
    button_num_white_move, button_num_black_move};
static const uint32_t LenB = 2;
static const uint32_t PinsB[] = {BUTTON_GPIO_PIN_UNDO_PIN,
                                 BUTTON_GPIO_PIN_PAUSE_PIN};
static const enum button_num ButtonsB[] = {button_num_undo, button_num_pause};

void GROUP1_IRQHandler(void) {
    uint32_t allPinsA = 0, allPinsB = 0;
    // TODO ensure optimizer precomputes this
    for (uint8_t i = 0; i < LenA; i++) {
        allPinsA |= PinsA[i];
    }
    for (uint8_t i = 0; i < LenB; i++) {
        allPinsB |= PinsB[i];
    }
    uint32_t gpioA = DL_GPIO_getEnabledInterruptStatus(GPIOA, allPinsA);
    uint32_t gpioB = DL_GPIO_getEnabledInterruptStatus(GPIOB, allPinsB);
// TODO ensure unrolled.
#pragma clang loop unroll(enable)
    for (uint8_t i = 0; i < LenA; i++) {
        if ((gpioA & PinsA[i]) == PinsA[i]) {
            xMain_button_press(ButtonsA[i]);
            DL_GPIO_clearInterruptStatus(GPIOA, PinsA[i]);
        }
    }
#pragma clang loop unroll(enable)
    for (uint8_t i = 0; i < LenB; i++) {
        if ((gpioB & PinsB[i]) == PinsB[i]) {
            xMain_button_press(ButtonsB[i]);
            DL_GPIO_clearInterruptStatus(GPIOB, PinsB[i]);
        }
    }
}

void vButton_Init(void) {
    NVIC_ClearPendingIRQ(BUTTON_GPIO_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(BUTTON_GPIO_GPIOA_INT_IRQN);
    NVIC_ClearPendingIRQ(BUTTON_GPIO_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(BUTTON_GPIO_GPIOB_INT_IRQN);
    DL_GPIO_clearPins(MISC_GPIO_PORT, MISC_GPIO_BUTTON_ENABLE_PIN);
}