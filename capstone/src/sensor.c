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

#define NBINS 13
// Based on our filter step responses, we should wait 150us for a column switch.
// (150 us * 32 MHz) - 1
#define COL_SWITCH_LOAD (150U * 64U / 10U - 1U)
// Similarly, 50us for a row switch.
#define ROW_SWITCH_LOAD (50U * 64U / 10U - 1U)

// For the ADC: ADC samples are asynchronous, so we need to wakeup the task from
// an ISR.
static TaskHandle_t xSensorTaskId = NULL;

// Lower bounds for hall effect sensor reading bins.
// Generate this with python (numpy):
// np.round((2**12-1)*np.array([0, 0.2, 0.6,
// 0.95, 1.1, 1.3, 1.55, 1.9, 2.1, 2.3, 2.4, 2.8, 3.0])/3.3).astype(int) OPT:
// make this packed 12-bit?
// TODO: calibration?
static uint16_t bins[] = {0,    173,  567,  962,  1332, 1708, 1968,
                          2128, 2387, 2763, 3133, 3528, 3923};

static PieceType prvValueToPiece(uint16_t value) {
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

static void prvSingleADC(BoardState *board, uint8_t row, uint8_t column) {
    DL_ADC12_startConversion(ADC_0_INST);
    // Block the thread until ADC sampling is complete.
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    volatile uint16_t sample =
        DL_ADC12_getMemResult(ADC_0_INST, ADC_0_ADCMEM_ChessSquare);
    vSetSquare(board, row, column, prvValueToPiece(sample));
    DL_ADC12_enableConversions(ADC_0_INST);
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

void SENSOR_DELAY_TIMER_INST_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    switch (DL_TimerG_getPendingInterrupt(SENSOR_DELAY_TIMER_INST)) {
    case DL_TIMER_IIDX_ZERO:
        vTaskNotifyGiveFromISR(xSensorTaskId, &xHigherPriorityTaskWoken);
        break;
    default:
        break;
    }

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
    // TODO: maybe wait to be prompted by a fifo? Maybe have a timer do that?
    // TODO: noise: don't read while LEDs are going?
    while (true) {
        // This is uninitialized, but that doesn't matter. Each element will be
        // initialized.
        BoardState board;
        for (uint8_t col = 0; col < 8; col++) {
            prvSelectColumn(col);
            // Set the timer for 150us delay.
            DL_TimerG_setLoadValue(SENSOR_DELAY_TIMER_INST, COL_SWITCH_LOAD);
            // Wait for the timer, at most waiting 1ms.
            DL_TimerG_startCounter(SENSOR_DELAY_TIMER_INST);
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
            // Reduce to the smaller time for switching between the rows.
            DL_TimerG_setLoadValue(SENSOR_DELAY_TIMER_INST, ROW_SWITCH_LOAD);
            for (uint8_t row = 0; row < 8; row++) {
                prvSelectRow(row);
                // Wait again for signal propagation.
                DL_TimerG_startCounter(SENSOR_DELAY_TIMER_INST);
                ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
                // Take the sample!
                prvSingleADC(&board, row, col);
            }
        }
        xMain_sensor_update(&board);
    }
}