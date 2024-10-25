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
    SYSCFG_DL_RPI_UART_init();
    SYSCFG_DL_LED_SPI_init();
    SYSCFG_DL_CLOCK_SPI_init();
    SYSCFG_DL_ADC_0_init();
}



SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_UART_Main_reset(RPI_UART_INST);
    DL_SPI_reset(LED_SPI_INST);
    DL_SPI_reset(CLOCK_SPI_INST);
    DL_ADC12_reset(ADC_0_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_UART_Main_enablePower(RPI_UART_INST);
    DL_SPI_enablePower(LED_SPI_INST);
    DL_SPI_enablePower(CLOCK_SPI_INST);
    DL_ADC12_enablePower(ADC_0_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_RPI_UART_IOMUX_TX, GPIO_RPI_UART_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_RPI_UART_IOMUX_RX, GPIO_RPI_UART_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_LED_SPI_IOMUX_SCLK, GPIO_LED_SPI_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_LED_SPI_IOMUX_PICO, GPIO_LED_SPI_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_CLOCK_SPI_IOMUX_SCLK, GPIO_CLOCK_SPI_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_CLOCK_SPI_IOMUX_PICO, GPIO_CLOCK_SPI_IOMUX_PICO_FUNC);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_R_A0_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_R_A1_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_R_A2_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_C_A0_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_C_A1_IOMUX);

    DL_GPIO_initDigitalOutput(MUX_GPIO_PIN_C_A2_IOMUX);

    DL_GPIO_initDigitalInputFeatures(BUTTON_GPIO_PIN_HINT_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(BUTTON_GPIO_PIN_UNDO_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(BUTTON_GPIO_PIN_START_RESTART_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(BUTTON_GPIO_PIN_PAUSE_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(BUTTON_GPIO_PIN_CLOCK_MODE_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(BUTTON_GPIO_PIN_BLACK_MOVE_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(BUTTON_GPIO_PIN_WHITE_MOVE_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(MISC_GPIO_CLOCK_LOAD_IOMUX);

    DL_GPIO_initDigitalOutput(MISC_GPIO_BUTTON_ENABLE_IOMUX);

    DL_GPIO_clearPins(GPIOA, MUX_GPIO_PIN_R_A2_PIN |
		MUX_GPIO_PIN_C_A0_PIN);
    DL_GPIO_enableOutput(GPIOA, MUX_GPIO_PIN_R_A2_PIN |
		MUX_GPIO_PIN_C_A0_PIN);
    DL_GPIO_setLowerPinsPolarity(GPIOA, DL_GPIO_PIN_15_EDGE_RISE);
    DL_GPIO_setUpperPinsPolarity(GPIOA, DL_GPIO_PIN_27_EDGE_RISE |
		DL_GPIO_PIN_17_EDGE_RISE |
		DL_GPIO_PIN_16_EDGE_RISE |
		DL_GPIO_PIN_24_EDGE_RISE);
    DL_GPIO_clearInterruptStatus(GPIOA, BUTTON_GPIO_PIN_HINT_PIN |
		BUTTON_GPIO_PIN_START_RESTART_PIN |
		BUTTON_GPIO_PIN_CLOCK_MODE_PIN |
		BUTTON_GPIO_PIN_BLACK_MOVE_PIN |
		BUTTON_GPIO_PIN_WHITE_MOVE_PIN);
    DL_GPIO_enableInterrupt(GPIOA, BUTTON_GPIO_PIN_HINT_PIN |
		BUTTON_GPIO_PIN_START_RESTART_PIN |
		BUTTON_GPIO_PIN_CLOCK_MODE_PIN |
		BUTTON_GPIO_PIN_BLACK_MOVE_PIN |
		BUTTON_GPIO_PIN_WHITE_MOVE_PIN);
    DL_GPIO_clearPins(GPIOB, MUX_GPIO_PIN_R_A0_PIN |
		MUX_GPIO_PIN_R_A1_PIN |
		MUX_GPIO_PIN_C_A1_PIN |
		MUX_GPIO_PIN_C_A2_PIN |
		MISC_GPIO_CLOCK_LOAD_PIN);
    DL_GPIO_setPins(GPIOB, MISC_GPIO_BUTTON_ENABLE_PIN);
    DL_GPIO_enableOutput(GPIOB, MUX_GPIO_PIN_R_A0_PIN |
		MUX_GPIO_PIN_R_A1_PIN |
		MUX_GPIO_PIN_C_A1_PIN |
		MUX_GPIO_PIN_C_A2_PIN |
		MISC_GPIO_CLOCK_LOAD_PIN |
		MISC_GPIO_BUTTON_ENABLE_PIN);
    DL_GPIO_setLowerPinsPolarity(GPIOB, DL_GPIO_PIN_2_EDGE_RISE |
		DL_GPIO_PIN_3_EDGE_RISE);
    DL_GPIO_clearInterruptStatus(GPIOB, BUTTON_GPIO_PIN_UNDO_PIN |
		BUTTON_GPIO_PIN_PAUSE_PIN);
    DL_GPIO_enableInterrupt(GPIOB, BUTTON_GPIO_PIN_UNDO_PIN |
		BUTTON_GPIO_PIN_PAUSE_PIN);

}


SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_1);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);
    /* INT_GROUP1 Priority */
    NVIC_SetPriority(GPIOA_INT_IRQn, 3);

}



static const DL_UART_Main_ClockConfig gRPI_UARTClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gRPI_UARTConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_RPI_UART_init(void)
{
    DL_UART_Main_setClockConfig(RPI_UART_INST, (DL_UART_Main_ClockConfig *) &gRPI_UARTClockConfig);

    DL_UART_Main_init(RPI_UART_INST, (DL_UART_Main_Config *) &gRPI_UARTConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115211.52
     */
    DL_UART_Main_setOversampling(RPI_UART_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(RPI_UART_INST, RPI_UART_IBRD_32_MHZ_115200_BAUD, RPI_UART_FBRD_32_MHZ_115200_BAUD);



    DL_UART_Main_enable(RPI_UART_INST);
}

static const DL_SPI_Config gLED_SPI_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_16,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gLED_SPI_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_LED_SPI_init(void) {
    DL_SPI_setClockConfig(LED_SPI_INST, (DL_SPI_ClockConfig *) &gLED_SPI_clockConfig);

    DL_SPI_init(LED_SPI_INST, (DL_SPI_Config *) &gLED_SPI_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     16000000 = (32000000)/((1 + 0) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(LED_SPI_INST, 0);
    /* Enable packing feature */
    DL_SPI_enablePacking(LED_SPI_INST);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(LED_SPI_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(LED_SPI_INST);
}
static const DL_SPI_Config gCLOCK_SPI_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA1,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_16,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gCLOCK_SPI_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_CLOCK_SPI_init(void) {
    DL_SPI_setClockConfig(CLOCK_SPI_INST, (DL_SPI_ClockConfig *) &gCLOCK_SPI_clockConfig);

    DL_SPI_init(CLOCK_SPI_INST, (DL_SPI_Config *) &gCLOCK_SPI_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     1000000 = (32000000)/((1 + 15) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(CLOCK_SPI_INST, 15);
    /* Enable packing feature */
    DL_SPI_enablePacking(CLOCK_SPI_INST);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(CLOCK_SPI_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(CLOCK_SPI_INST);
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
        DL_ADC12_INPUT_CHAN_2, DL_ADC12_REFERENCE_VOLTAGE_VDDA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_ENABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_configHwAverage(ADC_0_INST,DL_ADC12_HW_AVG_NUM_ACC_4,DL_ADC12_HW_AVG_DEN_DIV_BY_4);
    DL_ADC12_setSampleTime0(ADC_0_INST,2);
    /* Enable ADC12 interrupt */
    DL_ADC12_clearInterruptStatus(ADC_0_INST,(DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED));
    DL_ADC12_enableInterrupt(ADC_0_INST,(DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED));
    DL_ADC12_enableConversions(ADC_0_INST);
}

