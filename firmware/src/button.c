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
#include "private/button_private.h"
#include "portmacro.h"
#include "projdefs.h"
#include "ti/driverlib/dl_gpio.h"
#include "main.h"

static uint32_t allPinsA = 0, allPinsB = 0;

void GROUP1_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint32_t gpioA = DL_GPIO_getEnabledInterruptStatus(GPIOA, allPinsA);
    uint32_t gpioB = DL_GPIO_getEnabledInterruptStatus(GPIOB, allPinsB);

#pragma clang loop unroll(enable)
    for (uint8_t i = 0; i < PRVBUTTON_LENA; i++) {
        if ((gpioA & prvButton_PinsA[i]) == prvButton_PinsA[i]) {
            xMain_button_press_FromISR(prvButton_ButtonsA[i], &xHigherPriorityTaskWoken);
            DL_GPIO_clearInterruptStatus(GPIOA, prvButton_PinsA[i]);
        }
    }
#pragma clang loop unroll(enable)
    for (uint8_t i = 0; i < PRVBUTTON_LENB; i++) {
        if ((gpioB & prvButton_PinsB[i]) == prvButton_PinsB[i]) {
            xMain_button_press_FromISR(prvButton_ButtonsB[i], &xHigherPriorityTaskWoken);
            DL_GPIO_clearInterruptStatus(GPIOB, prvButton_PinsB[i]);
        }
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vButton_Init(void) {
    // Pre-compute this; it doesn't change over the program runtime.
    for (uint8_t i = 0; i < PRVBUTTON_LENA; i++) {
        allPinsA |= prvButton_PinsA[i];
    }
    for (uint8_t i = 0; i < PRVBUTTON_LENB; i++) {
        allPinsB |= prvButton_PinsB[i];
    }
    NVIC_ClearPendingIRQ(BUTTON_GPIO_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(BUTTON_GPIO_GPIOA_INT_IRQN);
    NVIC_ClearPendingIRQ(BUTTON_GPIO_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(BUTTON_GPIO_GPIOB_INT_IRQN);
    DL_GPIO_clearPins(MISC_GPIO_BUTTON_ENABLE_PORT, MISC_GPIO_BUTTON_ENABLE_PIN);
}
