/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * button.c: logic to handle pushbutton presses on the C.H.E.S.S.B.O.A.R.D.
 * and translate them into game events (play, hint, undo, clock toggle,
 * restart) for the firmware.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <https://www.gnu.org/licenses/>
 */
#include "button.h"
#include "config.h"
#include "portmacro.h"
#include "projdefs.h"
#include "ti/driverlib/dl_gpio.h"
#include "ti_msp_dl_config.h"
#include "main.h"

static const uint32_t LenA = 5;
static const uint32_t PinsA[] = {
    BUTTON_GPIO_PIN_PAUSE_PIN, BUTTON_GPIO_PIN_START_RESTART_PIN,
    BUTTON_GPIO_PIN_CLOCK_MODE_PIN, BUTTON_GPIO_PIN_BLACK_MOVE_PIN,
    BUTTON_GPIO_PIN_WHITE_MOVE_PIN};
static const enum button_num ButtonsA[] = {
    button_num_pause, button_num_start_restart, button_num_clock_mode,
    button_num_white_move, button_num_black_move};
static const uint32_t LenB = 2;
static const uint32_t PinsB[] = {BUTTON_GPIO_PIN_UNDO_PIN,
                                 BUTTON_GPIO_PIN_HINT_PIN};
static const enum button_num ButtonsB[] = {button_num_undo, button_num_hint};

void GROUP1_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t allPinsA = 0, allPinsB = 0;

    for (uint8_t i = 0; i < LenA; i++) {
        allPinsA |= PinsA[i];
    }
    for (uint8_t i = 0; i < LenB; i++) {
        allPinsB |= PinsB[i];
    }
    uint32_t gpioA = DL_GPIO_getEnabledInterruptStatus(GPIOA, allPinsA);
    uint32_t gpioB = DL_GPIO_getEnabledInterruptStatus(GPIOB, allPinsB);

#pragma clang loop unroll(enable)
    for (uint8_t i = 0; i < LenA; i++) {
        if ((gpioA & PinsA[i]) == PinsA[i]) {
            xMain_button_press_FromISR(ButtonsA[i], &xHigherPriorityTaskWoken);
            DL_GPIO_clearInterruptStatus(GPIOA, PinsA[i]);
        }
    }
#pragma clang loop unroll(enable)
    for (uint8_t i = 0; i < LenB; i++) {
        if ((gpioB & PinsB[i]) == PinsB[i]) {
            xMain_button_press_FromISR(ButtonsB[i], &xHigherPriorityTaskWoken);
            DL_GPIO_clearInterruptStatus(GPIOB, PinsB[i]);
        }
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vButton_Init(void) {
    NVIC_ClearPendingIRQ(BUTTON_GPIO_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(BUTTON_GPIO_GPIOA_INT_IRQN);
    NVIC_ClearPendingIRQ(BUTTON_GPIO_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(BUTTON_GPIO_GPIOB_INT_IRQN);
    DL_GPIO_clearPins(MISC_GPIO_BUTTON_ENABLE_PORT, MISC_GPIO_BUTTON_ENABLE_PIN);
}
