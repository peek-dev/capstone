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
/* Defines for PIN_MODE0: GPIOB.7 with pinCMx 24 on package pin 59 */
#define MUX_GPIO_PIN_MODE0_PORT                                          (GPIOB)
#define MUX_GPIO_PIN_MODE0_PIN                                   (DL_GPIO_PIN_7)
#define MUX_GPIO_PIN_MODE0_IOMUX                                 (IOMUX_PINCM24)
/* Defines for PIN_MODE1: GPIOA.12 with pinCMx 34 on package pin 5 */
#define MUX_GPIO_PIN_MODE1_PORT                                          (GPIOA)
#define MUX_GPIO_PIN_MODE1_PIN                                  (DL_GPIO_PIN_12)
#define MUX_GPIO_PIN_MODE1_IOMUX                                 (IOMUX_PINCM34)
/* Defines for PIN_REPOS: GPIOB.16 with pinCMx 33 on package pin 4 */
#define MUX_GPIO_PIN_REPOS_PORT                                          (GPIOB)
#define MUX_GPIO_PIN_REPOS_PIN                                  (DL_GPIO_PIN_16)
#define MUX_GPIO_PIN_REPOS_IOMUX                                 (IOMUX_PINCM33)
/* Port definition for Pin Group LED_GPIO */
#define LED_GPIO_PORT                                                    (GPIOA)

/* Defines for LED1: GPIOA.0 with pinCMx 1 on package pin 33 */
#define LED_GPIO_LED1_PIN                                        (DL_GPIO_PIN_0)
#define LED_GPIO_LED1_IOMUX                                       (IOMUX_PINCM1)

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_ADC_0_init(void);



#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
