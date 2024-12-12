#include "calibration.h"
#include "config.h"
#include "portmacro.h"
#include "task.h"
#include "assert.h"
#include "ti/driverlib/dl_adc12.h"
#include "ti/driverlib/dl_gpio.h"
#include "ti/driverlib/dl_timerg.h"

#include "game.h"
#include "sensor.h"
#include "main.h"
#include "ti_msp_dl_config.h"
#include "semphr.h"
#include "util.h"

#define SENSOR_DELAY_MS 20
#define NBINS           13
// Similarly, 50us for a row switch.
#define SWITCH_DELAY_MS 10

// For the ADC: ADC samples are asynchronous, so we need to wakeup the task from
// an ISR.
static TaskHandle_t xSensorTaskId = NULL;

SemaphoreHandle_t sensor_mutex;

static PieceType prvValueToPiece(uint16_t value, const uint16_t *bins) {
    uint8_t i;
    for (i = 0; i < NBINS; i++) {
        if (bins[i] > value) {
            break;
        }
    }
    // TODO confirm that this matches the correct PieceType enum order.
    return (PieceType)(i - 1);
}

static void prvSelectRow(uint8_t row) {
    assert(row < 8);
    // Hack: all rows were reversed.
    row = 7 - row;
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_R_A0_PORT, MUX_GPIO_PIN_R_A0_PIN,
                         MUX_GPIO_PIN_R_A0_PIN * !!(row & 1));
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_R_A1_PORT, MUX_GPIO_PIN_R_A1_PIN,
                         MUX_GPIO_PIN_R_A1_PIN * !!(row & 2));
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_R_A2_PORT, MUX_GPIO_PIN_R_A2_PIN,
                         MUX_GPIO_PIN_R_A2_PIN * !!(row & 4));
}

static void prvSelectColumn(uint8_t column) {
    assert(column < 8);
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_C_A0_PORT, MUX_GPIO_PIN_C_A0_PIN,
                         MUX_GPIO_PIN_C_A0_PIN *
                             (!!(column & 1) ^ !!(column & 4)));
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_C_A1_PORT, MUX_GPIO_PIN_C_A1_PIN,
                         MUX_GPIO_PIN_C_A1_PIN *
                             (!!(column & 2) ^ !!(column & 4)));
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_C_A2_PORT, MUX_GPIO_PIN_C_A2_PIN,
                         MUX_GPIO_PIN_C_A2_PIN * !!(column & 4));
}

static uint16_t prvSingleADC() {
    DL_ADC12_startConversion(ADC_0_INST);
    // Block the thread until ADC sampling is complete.
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    uint16_t sample =
        DL_ADC12_getMemResult(ADC_0_INST, ADC_0_ADCMEM_ChessSquare);
    DL_ADC12_enableConversions(ADC_0_INST);
    return sample;
}

void ADC_0_INST_IRQHandler(void) {
    // I feel like we should be clearing an interrupt thing here, but the
    // example didn't, sooo...
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    switch (DL_ADC12_getPendingInterrupt(ADC_0_INST)) {
    case DL_ADC12_IIDX_MEM0_RESULT_LOADED:
        vTaskNotifyGiveFromISR(xSensorTaskId, &xHigherPriorityTaskWoken);
        break;
    default:
        break;
    }

    // If we woke up a higher-priority task than the one currently running,
    // we should let the scheduler know so that it can switch.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


BaseType_t xSensor_Init(void) {
    prvSelectColumn(0);
    prvSelectRow(0);
    NVIC_ClearPendingIRQ(SENSOR_DELAY_TIMER_INST_INT_IRQN);
    NVIC_EnableIRQ(SENSOR_DELAY_TIMER_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(ADC_0_INST_INT_IRQN);
    NVIC_EnableIRQ(ADC_0_INST_INT_IRQN);
    return pdTRUE;
}

void vSensor_Thread(void *arg0) {
    assert(xSensorTaskId == NULL);
    xSensorTaskId = xTaskGetCurrentTaskHandle();

    while (true) {
        // This is uninitialized, but that doesn't matter. Each element will be
        // initialized.
        BoardState board;
        uint16_t samples[5];
        for (uint8_t col = 0; col < 8; col++) {
            prvSelectColumn(col);
            for (uint8_t row = 0; row < 8; row++) {
                prvSelectRow(row);
                // Wait for signal propagation.
                vTaskDelay(SWITCH_DELAY_MS);
                // Take the samples!
                for (uint8_t i = 0; i < 5; i++) {
                    samples[i] = prvSingleADC();
                }
                uint16_t sample = MedianOfFive(samples);
                vSetSquare(&board, row, col,
                           prvValueToPiece(sample, GetBins(row, col)));
            }
        }
        xMain_sensor_update(&board);
        vTaskDelay(SENSOR_DELAY_MS / portTICK_PERIOD_MS);
    }
}
