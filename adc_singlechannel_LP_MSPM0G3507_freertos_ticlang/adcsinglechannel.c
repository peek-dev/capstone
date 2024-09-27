/*
 * Copyright (c) 2018-2023, Texas Instruments Incorporated
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
 *  ======== adcsinglechannel.c ========
 */
#include <string.h>
#include <stddef.h>
#include <stdint.h>

/* FreeRTOS Headers*/
#include <FreeRTOS.h>
#include <queue.h>

/* Driver Header files */
#include <ti/display/DisplayUart.h>
#include <ti/drivers/ADC.h>
#include <ti/drivers/GPIO.h>

/* Driver configuration */
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "projdefs.h"
#include "ti/display/Display.h"
#include "ti_drivers_config.h"

/* ADC sample count */
#define ADC_SAMPLE_COUNT (10)

#define THREADSTACKSIZE (768)
#define MSG_STR_SIZE (32)
#define QUEUE_SIZE 2

/* ADC conversion result variables */
uint16_t adcValue0;
uint32_t adcValue0MicroVolt;
uint16_t adcValue1[ADC_SAMPLE_COUNT];
uint32_t adcValue1MicroVolt[ADC_SAMPLE_COUNT];

static Display_Handle display;
static QueueHandle_t displayQueue;

typedef struct msg_struct {
    char fmtStr[MSG_STR_SIZE];
    uint32_t param1;
    uint32_t param2;
} DisplayMsg;
#if QUEUE_STATIC
static uint8_t queue_storage[QUEUE_SIZE*sizeof(DisplayMsg)];
static StaticQueue_t queue_struct;
#endif

void send_printf(char *fmt, uint32_t param1, uint32_t param2);
/*
 *  ======== threadFxn0 ========
 *  Open an ADC instance and get a sampling result from a one-shot conversion.
 */
void threadFxn0(void *arg0)
{
    ADC_Handle adc;
    ADC_Params params;
    int_fast16_t res;

    ADC_Params_init(&params);
    adc = ADC_open(CONFIG_ADC_0, &params);

    if (adc == NULL) {
        send_printf("Error initializing A0\n", 0, 0);
        while (1) {
        }
    }

    /* Blocking mode conversion */
    res = ADC_convert(adc, &adcValue0);

    if (res == ADC_STATUS_SUCCESS) {
        adcValue0MicroVolt = ADC_convertRawToMicroVolts(adc, adcValue0);

        send_printf("A0 raw: %d\n", adcValue0, 0);
        send_printf("A0 convert: %d uV\n", adcValue0MicroVolt, 0);
    } else {
        send_printf("A0 failed\n", 0, 0);
    }

    ADC_close(adc);
    vTaskDelete(NULL);
}

/*
 *  ======== threadFxn1 ========
 *  Open a ADC handle and get an array of sampling results after
 *  calling several conversions.
 */
void threadFxn1(void *arg0)
{
    uint16_t i;
    ADC_Handle adc;
    ADC_Params params;
    int_fast16_t res;

    ADC_Params_init(&params);
    adc = ADC_open(CONFIG_ADC_1, &params);

    if (adc == NULL) {
        send_printf("Error initializing A1\n", 0, 0);
        while (1) {
        }
    }

    for (i = 0; i < ADC_SAMPLE_COUNT; i++) {
        res = ADC_convert(adc, &adcValue1[i]);

        if (res == ADC_STATUS_SUCCESS) {
            adcValue1MicroVolt[i] = ADC_convertToMicroVolts(adc, adcValue1[i]);

            send_printf("A1 raw (%d): %d\n", i, adcValue1[i]);
            send_printf("A1 convert (%d): %d uV\n", i, adcValue1MicroVolt[i]);
        } else {
            send_printf("A1 failed (%d)\n", i, 0);
        }
    }

    ADC_close(adc);
    vTaskDelete(NULL);
}

void send_printf(char *fmt, uint32_t param1, uint32_t param2) {
    DisplayMsg m;
    m.param1 = param1;
    m.param2 = param2;
    strncpy(&m.fmtStr[0], fmt, MSG_STR_SIZE);
    xQueueSend(displayQueue, &m, portMAX_DELAY);
}

void msgThread(void *arg0) {
    DisplayMsg message;
    while (1) {
        if (xQueueReceive(displayQueue, &message, portMAX_DELAY) == pdTRUE) {
            Display_printf(display, 0, 0, message.fmtStr, message.param1, message.param2);
        }
    }
}

/*
 *  ======== mainThread ========
 */
void mainThread(void *arg0)
{
    TaskHandle_t thread0, thread1, threadm;
    BaseType_t xReturned;

    /* Call driver init functions */
    Display_init();
    GPIO_init();
    ADC_init();

    /* Configure the LED and if applicable, the TMP_EN pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0,
        GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH | CONFIG_GPIO_LED_0_IOMUX);
    
    // Initialize the queue
#if QUEUE_STATIC
    displayQueue = xQueueCreateStatic(QUEUE_SIZE, sizeof(DisplayMsg), queue_storage, &queue_struct);
#else
    displayQueue = xQueueCreate(QUEUE_SIZE, sizeof(DisplayMsg));
#endif
    if (displayQueue == NULL) {
        while (1) {}
    }

    /* Open the UART display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        /* Failed to open display driver */
        while (1) {
        }
    }

    /* Turn on user LED to indicate successful initialization */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_LED_ON);
    Display_printf(display, 0, 0, "Starting the adcsinglechannel example\n");

    // Make a thread to recieve messages.
    xReturned = xTaskCreate(msgThread, "Message", 2*configMINIMAL_STACK_SIZE, NULL, 2, &threadm);
    // Loop and get stuck here if we fail.
    while (xReturned != pdPASS) {}
    xReturned = xPortGetFreeHeapSize();

    // The first ADC thread.
    xReturned = xTaskCreate(threadFxn0, "ADC0", configMINIMAL_STACK_SIZE, NULL, 1, &thread0);
    while (xReturned != pdPASS) {}
    xReturned = xPortGetFreeHeapSize();
    // The second ADC thread.
    xReturned = xTaskCreate(threadFxn1, "ADC1", configMINIMAL_STACK_SIZE, NULL, 1, &thread1);
    while (xReturned != pdPASS) {}
    xReturned = xPortGetFreeHeapSize();
    vTaskDelete(NULL);
}
