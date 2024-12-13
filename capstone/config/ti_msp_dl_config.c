/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the LP_MSPM0G3507
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_ADC_0_init();
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_ADC12_reset(ADC_0_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_ADC12_enablePower(ADC_0_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_R_A0_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_R_A1_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_R_A2_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_C_A0_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_C_A1_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_C_A2_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_MODE0_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_MODE1_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_REPOS_IOMUX);

    DL_GPIO_initDigitalOutput(LED_GPIO_LED1_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_TOGGLE_DEV_IOMUX);

    DL_GPIO_clearPins(GPIOA, MUX_GPIO_PIN_R_A2_PIN);
    DL_GPIO_setPins(GPIOA, MUX_GPIO_PIN_C_A0_PIN |
		MUX_GPIO_PIN_MODE1_PIN |
		LED_GPIO_LED1_PIN);
    DL_GPIO_enableOutput(GPIOA, MUX_GPIO_PIN_R_A2_PIN |
		MUX_GPIO_PIN_C_A0_PIN |
		MUX_GPIO_PIN_MODE1_PIN |
		LED_GPIO_LED1_PIN);
    DL_GPIO_clearPins(GPIOB, MUX_GPIO_PIN_R_A0_PIN |
		MUX_GPIO_PIN_R_A1_PIN |
		MUX_GPIO_PIN_C_A1_PIN |
		MUX_GPIO_PIN_C_A2_PIN |
		MUX_GPIO_PIN_REPOS_PIN |
		MUX_GPIO_PIN_TOGGLE_DEV_PIN);
    DL_GPIO_setPins(GPIOB, MUX_GPIO_PIN_MODE0_PIN);
    DL_GPIO_enableOutput(GPIOB, MUX_GPIO_PIN_R_A0_PIN |
		MUX_GPIO_PIN_R_A1_PIN |
		MUX_GPIO_PIN_C_A1_PIN |
		MUX_GPIO_PIN_C_A2_PIN |
		MUX_GPIO_PIN_MODE0_PIN |
		MUX_GPIO_PIN_REPOS_PIN |
		MUX_GPIO_PIN_TOGGLE_DEV_PIN);

}


SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_1);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

}


/* ADC_0 Initialization */
static const DL_ADC12_ClockConfig gADC_0ClockConfig = {
    .clockSel       = DL_ADC12_CLOCK_ULPCLK,
    .divideRatio    = DL_ADC12_CLOCK_DIVIDE_1,
    .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
};
SYSCONFIG_WEAK void SYSCFG_DL_ADC_0_init(void)
{
    DL_ADC12_setClockConfig(ADC_0_INST, (DL_ADC12_ClockConfig *) &gADC_0ClockConfig);
    DL_ADC12_configConversionMem(ADC_0_INST, ADC_0_ADCMEM_ChessSquare,
        DL_ADC12_INPUT_CHAN_2, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_setPowerDownMode(ADC_0_INST,DL_ADC12_POWER_DOWN_MODE_MANUAL);
    DL_ADC12_setSampleTime0(ADC_0_INST,32);
    /* Enable ADC12 interrupt */
    DL_ADC12_clearInterruptStatus(ADC_0_INST,(DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED));
    DL_ADC12_enableInterrupt(ADC_0_INST,(DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED));
    DL_ADC12_enableConversions(ADC_0_INST);
}

