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



/* Defines for RPI_UART */
#define RPI_UART_INST                                                      UART1
#define RPI_UART_INST_FREQUENCY                                         32000000
#define RPI_UART_INST_IRQHandler                                UART1_IRQHandler
#define RPI_UART_INST_INT_IRQN                                    UART1_INT_IRQn
#define GPIO_RPI_UART_RX_PORT                                              GPIOA
#define GPIO_RPI_UART_TX_PORT                                              GPIOA
#define GPIO_RPI_UART_RX_PIN                                       DL_GPIO_PIN_9
#define GPIO_RPI_UART_TX_PIN                                       DL_GPIO_PIN_8
#define GPIO_RPI_UART_IOMUX_RX                                   (IOMUX_PINCM20)
#define GPIO_RPI_UART_IOMUX_TX                                   (IOMUX_PINCM19)
#define GPIO_RPI_UART_IOMUX_RX_FUNC                    IOMUX_PINCM20_PF_UART1_RX
#define GPIO_RPI_UART_IOMUX_TX_FUNC                    IOMUX_PINCM19_PF_UART1_TX
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
#define GPIO_CLOCK_SPI_SCLK_PIN                                   DL_GPIO_PIN_9
#define GPIO_CLOCK_SPI_IOMUX_SCLK                               (IOMUX_PINCM26)
#define GPIO_CLOCK_SPI_IOMUX_SCLK_FUNC               IOMUX_PINCM26_PF_SPI1_SCLK



/* Defines for ADC_0 */
#define ADC_0_INST                                                          ADC0
#define ADC_0_INST_IRQHandler                                    ADC0_IRQHandler
#define ADC_0_INST_INT_IRQN                                      (ADC0_INT_IRQn)
#define ADC_0_ADCMEM_7                                        DL_ADC12_MEM_IDX_0
#define ADC_0_ADCMEM_7_REF                       DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC_0_ADCMEM_7_REF_VOLTAGE_V                                         3.3
#define GPIO_ADC_0_C7_PORT                                                 GPIOA
#define GPIO_ADC_0_C7_PIN                                         DL_GPIO_PIN_22


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
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