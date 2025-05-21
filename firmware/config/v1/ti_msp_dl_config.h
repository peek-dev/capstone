/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the LP_MSPM0G3507
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_LP_MSPM0G3507
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define CPUCLK_FREQ                                                     32000000



/* Defines for SENSOR_DELAY_TIMER */
#define SENSOR_DELAY_TIMER_INST                                          (TIMG0)
#define SENSOR_DELAY_TIMER_INST_IRQHandler                        TIMG0_IRQHandler
#define SENSOR_DELAY_TIMER_INST_INT_IRQN                        (TIMG0_INT_IRQn)
#define SENSOR_DELAY_TIMER_INST_LOAD_VALUE                                  (959U)
/* Defines for LCD_DELAY_LOAD */
#define LCD_DELAY_LOAD_INST                                              (TIMG6)
#define LCD_DELAY_LOAD_INST_IRQHandler                          TIMG6_IRQHandler
#define LCD_DELAY_LOAD_INST_INT_IRQN                            (TIMG6_INT_IRQn)
#define LCD_DELAY_LOAD_INST_LOAD_VALUE                                     (31U)



/* Defines for RPI_UART */
#define RPI_UART_INST                                                      UART1
#define RPI_UART_INST_FREQUENCY                                         32000000
#define RPI_UART_INST_IRQHandler                                UART1_IRQHandler
#define RPI_UART_INST_INT_IRQN                                    UART1_INT_IRQn
#define GPIO_RPI_UART_RX_PORT                                              GPIOB
#define GPIO_RPI_UART_TX_PORT                                              GPIOB
#define GPIO_RPI_UART_RX_PIN                                       DL_GPIO_PIN_7
#define GPIO_RPI_UART_TX_PIN                                       DL_GPIO_PIN_6
#define GPIO_RPI_UART_IOMUX_RX                                   (IOMUX_PINCM24)
#define GPIO_RPI_UART_IOMUX_TX                                   (IOMUX_PINCM23)
#define GPIO_RPI_UART_IOMUX_RX_FUNC                    IOMUX_PINCM24_PF_UART1_RX
#define GPIO_RPI_UART_IOMUX_TX_FUNC                    IOMUX_PINCM23_PF_UART1_TX
#define RPI_UART_BAUD_RATE                                              (115200)
#define RPI_UART_IBRD_32_MHZ_115200_BAUD                                    (17)
#define RPI_UART_FBRD_32_MHZ_115200_BAUD                                    (23)




/* Defines for LED_SPI */
#define LED_SPI_INST                                                       SPI0
#define LED_SPI_INST_IRQHandler                                 SPI0_IRQHandler
#define LED_SPI_INST_INT_IRQN                                     SPI0_INT_IRQn
#define GPIO_LED_SPI_PICO_PORT                                            GPIOB
#define GPIO_LED_SPI_PICO_PIN                                    DL_GPIO_PIN_17
#define GPIO_LED_SPI_IOMUX_PICO                                 (IOMUX_PINCM43)
#define GPIO_LED_SPI_IOMUX_PICO_FUNC                 IOMUX_PINCM43_PF_SPI0_PICO
/* GPIO configuration for LED_SPI */
#define GPIO_LED_SPI_SCLK_PORT                                            GPIOA
#define GPIO_LED_SPI_SCLK_PIN                                    DL_GPIO_PIN_12
#define GPIO_LED_SPI_IOMUX_SCLK                                 (IOMUX_PINCM34)
#define GPIO_LED_SPI_IOMUX_SCLK_FUNC                 IOMUX_PINCM34_PF_SPI0_SCLK
/* Defines for CLOCK_SPI */
#define CLOCK_SPI_INST                                                     SPI1
#define CLOCK_SPI_INST_IRQHandler                               SPI1_IRQHandler
#define CLOCK_SPI_INST_INT_IRQN                                   SPI1_INT_IRQn
#define GPIO_CLOCK_SPI_PICO_PORT                                          GPIOB
#define GPIO_CLOCK_SPI_PICO_PIN                                   DL_GPIO_PIN_8
#define GPIO_CLOCK_SPI_IOMUX_PICO                               (IOMUX_PINCM25)
#define GPIO_CLOCK_SPI_IOMUX_PICO_FUNC               IOMUX_PINCM25_PF_SPI1_PICO
/* GPIO configuration for CLOCK_SPI */
#define GPIO_CLOCK_SPI_SCLK_PORT                                          GPIOB
#define GPIO_CLOCK_SPI_SCLK_PIN                                  DL_GPIO_PIN_16
#define GPIO_CLOCK_SPI_IOMUX_SCLK                               (IOMUX_PINCM33)
#define GPIO_CLOCK_SPI_IOMUX_SCLK_FUNC               IOMUX_PINCM33_PF_SPI1_SCLK



/* Defines for ADC_0 */
#define ADC_0_INST                                                          ADC0
#define ADC_0_INST_IRQHandler                                    ADC0_IRQHandler
#define ADC_0_INST_INT_IRQN                                      (ADC0_INT_IRQn)
#define ADC_0_ADCMEM_ChessSquare                              DL_ADC12_MEM_IDX_0
#define ADC_0_ADCMEM_ChessSquare_REF             DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC_0_ADCMEM_ChessSquare_REF_VOLTAGE_V                                     3.3
#define GPIO_ADC_0_C2_PORT                                                 GPIOA
#define GPIO_ADC_0_C2_PIN                                         DL_GPIO_PIN_25



/* Defines for PIN_R_A0: GPIOB.4 with pinCMx 17 on package pin 52 */
#define MUX_GPIO_PIN_R_A0_PORT                                           (GPIOB)
#define MUX_GPIO_PIN_R_A0_PIN                                    (DL_GPIO_PIN_4)
#define MUX_GPIO_PIN_R_A0_IOMUX                                  (IOMUX_PINCM17)
/* Defines for PIN_R_A1: GPIOB.1 with pinCMx 13 on package pin 48 */
#define MUX_GPIO_PIN_R_A1_PORT                                           (GPIOB)
#define MUX_GPIO_PIN_R_A1_PIN                                    (DL_GPIO_PIN_1)
#define MUX_GPIO_PIN_R_A1_IOMUX                                  (IOMUX_PINCM13)
/* Defines for PIN_R_A2: GPIOA.28 with pinCMx 3 on package pin 35 */
#define MUX_GPIO_PIN_R_A2_PORT                                           (GPIOA)
#define MUX_GPIO_PIN_R_A2_PIN                                   (DL_GPIO_PIN_28)
#define MUX_GPIO_PIN_R_A2_IOMUX                                   (IOMUX_PINCM3)
/* Defines for PIN_C_A0: GPIOA.31 with pinCMx 6 on package pin 39 */
#define MUX_GPIO_PIN_C_A0_PORT                                           (GPIOA)
#define MUX_GPIO_PIN_C_A0_PIN                                   (DL_GPIO_PIN_31)
#define MUX_GPIO_PIN_C_A0_IOMUX                                   (IOMUX_PINCM6)
/* Defines for PIN_C_A1: GPIOB.20 with pinCMx 48 on package pin 19 */
#define MUX_GPIO_PIN_C_A1_PORT                                           (GPIOB)
#define MUX_GPIO_PIN_C_A1_PIN                                   (DL_GPIO_PIN_20)
#define MUX_GPIO_PIN_C_A1_IOMUX                                  (IOMUX_PINCM48)
/* Defines for PIN_C_A2: GPIOB.13 with pinCMx 30 on package pin 1 */
#define MUX_GPIO_PIN_C_A2_PORT                                           (GPIOB)
#define MUX_GPIO_PIN_C_A2_PIN                                   (DL_GPIO_PIN_13)
#define MUX_GPIO_PIN_C_A2_IOMUX                                  (IOMUX_PINCM30)
/* Defines for PIN_HINT: GPIOA.27 with pinCMx 60 on package pin 31 */
#define BUTTON_GPIO_PIN_HINT_PORT                                        (GPIOA)
// pins affected by this interrupt request:["PIN_HINT","PIN_START_RESTART","PIN_CLOCK_MODE","PIN_BLACK_MOVE","PIN_WHITE_MOVE"]
#define BUTTON_GPIO_GPIOA_INT_IRQN                              (GPIOA_INT_IRQn)
#define BUTTON_GPIO_GPIOA_INT_IIDX              (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define BUTTON_GPIO_PIN_HINT_IIDX                           (DL_GPIO_IIDX_DIO27)
#define BUTTON_GPIO_PIN_HINT_PIN                                (DL_GPIO_PIN_27)
#define BUTTON_GPIO_PIN_HINT_IOMUX                               (IOMUX_PINCM60)
/* Defines for PIN_UNDO: GPIOB.2 with pinCMx 15 on package pin 50 */
#define BUTTON_GPIO_PIN_UNDO_PORT                                        (GPIOB)
// pins affected by this interrupt request:["PIN_UNDO","PIN_PAUSE"]
#define BUTTON_GPIO_GPIOB_INT_IRQN                              (GPIOB_INT_IRQn)
#define BUTTON_GPIO_GPIOB_INT_IIDX              (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define BUTTON_GPIO_PIN_UNDO_IIDX                            (DL_GPIO_IIDX_DIO2)
#define BUTTON_GPIO_PIN_UNDO_PIN                                 (DL_GPIO_PIN_2)
#define BUTTON_GPIO_PIN_UNDO_IOMUX                               (IOMUX_PINCM15)
/* Defines for PIN_START_RESTART: GPIOA.17 with pinCMx 39 on package pin 10 */
#define BUTTON_GPIO_PIN_START_RESTART_PORT                               (GPIOA)
#define BUTTON_GPIO_PIN_START_RESTART_IIDX                  (DL_GPIO_IIDX_DIO17)
#define BUTTON_GPIO_PIN_START_RESTART_PIN                       (DL_GPIO_PIN_17)
#define BUTTON_GPIO_PIN_START_RESTART_IOMUX                      (IOMUX_PINCM39)
/* Defines for PIN_PAUSE: GPIOB.3 with pinCMx 16 on package pin 51 */
#define BUTTON_GPIO_PIN_PAUSE_PORT                                       (GPIOB)
#define BUTTON_GPIO_PIN_PAUSE_IIDX                           (DL_GPIO_IIDX_DIO3)
#define BUTTON_GPIO_PIN_PAUSE_PIN                                (DL_GPIO_PIN_3)
#define BUTTON_GPIO_PIN_PAUSE_IOMUX                              (IOMUX_PINCM16)
/* Defines for PIN_CLOCK_MODE: GPIOA.16 with pinCMx 38 on package pin 9 */
#define BUTTON_GPIO_PIN_CLOCK_MODE_PORT                                  (GPIOA)
#define BUTTON_GPIO_PIN_CLOCK_MODE_IIDX                     (DL_GPIO_IIDX_DIO16)
#define BUTTON_GPIO_PIN_CLOCK_MODE_PIN                          (DL_GPIO_PIN_16)
#define BUTTON_GPIO_PIN_CLOCK_MODE_IOMUX                         (IOMUX_PINCM38)
/* Defines for PIN_BLACK_MOVE: GPIOA.24 with pinCMx 54 on package pin 25 */
#define BUTTON_GPIO_PIN_BLACK_MOVE_PORT                                  (GPIOA)
#define BUTTON_GPIO_PIN_BLACK_MOVE_IIDX                     (DL_GPIO_IIDX_DIO24)
#define BUTTON_GPIO_PIN_BLACK_MOVE_PIN                          (DL_GPIO_PIN_24)
#define BUTTON_GPIO_PIN_BLACK_MOVE_IOMUX                         (IOMUX_PINCM54)
/* Defines for PIN_WHITE_MOVE: GPIOA.15 with pinCMx 37 on package pin 8 */
#define BUTTON_GPIO_PIN_WHITE_MOVE_PORT                                  (GPIOA)
#define BUTTON_GPIO_PIN_WHITE_MOVE_IIDX                     (DL_GPIO_IIDX_DIO15)
#define BUTTON_GPIO_PIN_WHITE_MOVE_PIN                          (DL_GPIO_PIN_15)
#define BUTTON_GPIO_PIN_WHITE_MOVE_IOMUX                         (IOMUX_PINCM37)
/* Defines for CLOCK_LOAD: GPIOB.0 with pinCMx 12 on package pin 47 */
#define MISC_GPIO_CLOCK_LOAD_PORT                                        (GPIOB)
#define MISC_GPIO_CLOCK_LOAD_PIN                                 (DL_GPIO_PIN_0)
#define MISC_GPIO_CLOCK_LOAD_IOMUX                               (IOMUX_PINCM12)
/* Defines for BUTTON_ENABLE: GPIOB.9 with pinCMx 26 on package pin 61 */
#define MISC_GPIO_BUTTON_ENABLE_PORT                                     (GPIOB)
#define MISC_GPIO_BUTTON_ENABLE_PIN                              (DL_GPIO_PIN_9)
#define MISC_GPIO_BUTTON_ENABLE_IOMUX                            (IOMUX_PINCM26)
/* Defines for POWER_LED: GPIOA.0 with pinCMx 1 on package pin 33 */
#define MISC_GPIO_POWER_LED_PORT                                         (GPIOA)
#define MISC_GPIO_POWER_LED_PIN                                  (DL_GPIO_PIN_0)
#define MISC_GPIO_POWER_LED_IOMUX                                 (IOMUX_PINCM1)

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SENSOR_DELAY_TIMER_init(void);
void SYSCFG_DL_LCD_DELAY_LOAD_init(void);
void SYSCFG_DL_RPI_UART_init(void);
void SYSCFG_DL_LED_SPI_init(void);
void SYSCFG_DL_CLOCK_SPI_init(void);
void SYSCFG_DL_ADC_0_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
