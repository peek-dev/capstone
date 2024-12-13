#include "calibration.h"
#include "config.h"
#include "portmacro.h"
#include "projdefs.h"
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

#define SENSOR_DELAY_MS 0
#define NBINS           13
// Similarly, 50us for a row switch.
#define SWITCH_DELAY_MS 10
#define N_TRIALS        4

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

static void prvSelectMode(uint8_t mode) {
    assert(mode < 4);
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_MODE0_PORT, MUX_GPIO_PIN_MODE0_PIN,
                         MUX_GPIO_PIN_MODE0_PIN * !!(mode & 1));
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_MODE1_PORT, MUX_GPIO_PIN_MODE1_PIN,
                         MUX_GPIO_PIN_MODE1_PIN * !!(mode & 2));
}

static void prvSelectReposition(BaseType_t on) {
    DL_GPIO_writePinsVal(MUX_GPIO_PIN_REPOS_PORT, MUX_GPIO_PIN_REPOS_PIN,
                         MUX_GPIO_PIN_REPOS_PIN * (on == pdTRUE));
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
    NVIC_ClearPendingIRQ(ADC_0_INST_INT_IRQN);
    NVIC_EnableIRQ(ADC_0_INST_INT_IRQN);
    return pdTRUE;
}

uint16_t usSampleFiveMedian() {
    uint16_t samples[5];
    // Take the samples!
    for (uint8_t i = 0; i < 5; i++) {
        samples[i] = prvSingleADC();
    }
    return MedianOfFive(samples);
}

void vDoTripleSensor(uint16_t *sensor_values) {
    // For each of the three redundant sensors:
    for (uint8_t s_num = 0; s_num < 3; s_num++) {
        prvSelectMode(s_num + 1);
        // Wait for signal propagation.
        vTaskDelay(SWITCH_DELAY_MS);
        sensor_values[s_num] = usSampleFiveMedian();
    }
}

void vSensor_Thread(void *arg0) {
    assert(xSensorTaskId == NULL);
    xSensorTaskId = xTaskGetCurrentTaskHandle();
    for (uint8_t stddev_i = 0; stddev_i < N_STDDEV_BINS; stddev_i++) {
        prvSelectColumn(0);
        prvSelectRow(0);
        prvSelectReposition(pdFALSE);
        prvSelectMode(0);
        for (uint16_t count = 0; count < N_TRIALS; count++) {
            uint16_t repositions = 0;
            // This is uninitialized, but that doesn't matter. Each element will
            // be initialized.
            BoardState board, board_avg, board_weighted, board_request;
            uint16_t sensor_values[3];
            uint16_t sample;
            for (uint8_t col = 0; col < 8; col++) {
                prvSelectColumn(col);
                for (uint8_t row = 0; row < 8; row++) {
                    prvSelectRow(row);

                    // Single sensor mode.
                    prvSelectMode(0);
                    vTaskDelay(SWITCH_DELAY_MS);
                    sample = usSampleFiveMedian();
                    vSetSquare(&board, row, col, prvValueToPiece(sample, bins));

                    // There are three triple sensor modes.
                    vDoTripleSensor(sensor_values);
                    uint16_t sum_vals = sum(sensor_values, 3);
                    // The averaging one is the easiest.
                    PieceType p = prvValueToPiece(sum_vals, offset_bins_triple);
                    vSetSquare(&board_avg, row, col, p);

                    // Next comes weighted average.
                    // Find the outlier, and add it a fourth time to the offset.
                    uint16_t sum_weighted =
                        sum_vals + outlier(sensor_values, 3);
                    vSetSquare(&board_weighted, row, col,
                               prvValueToPiece(sum_weighted, offset_bins_quad));

                    // Finally, the re-request method.
                    if (prvValueToPiece(sensor_values[0], offset_bins) != p ||
                        prvValueToPiece(sensor_values[1], offset_bins) != p ||
                        prvValueToPiece(sensor_values[2], offset_bins) != p) {
                        // Request a reposition and rerun.
                        prvSelectReposition(pdTRUE);
                        vDoTripleSensor(sensor_values);
                        uint16_t sum_repos = sum(sensor_values, 3);
                        vSetSquare(
                            &board_request, row, col,
                            prvValueToPiece(sum_repos, offset_bins_triple));
                        repositions++;
                        prvSelectReposition(pdFALSE);
                    } else {
                        vSetSquare(&board_request, row, col, p);
                    }
                }
            }
            xMain_change_mode(main_thread_single, stddev_i);
            xMain_sensor_update(&board, 0);
            xMain_change_mode(main_thread_average, stddev_i);
            xMain_sensor_update(&board_avg, 0);
            xMain_change_mode(main_thread_weighted, stddev_i);
            xMain_sensor_update(&board_weighted, 0);
            xMain_change_mode(main_thread_requests, stddev_i);
            xMain_sensor_update(&board_request, repositions);
            vTaskDelay(SENSOR_DELAY_MS / portTICK_PERIOD_MS);
        }
    }
    xMain_change_mode(main_thread_finished, 0);
    vTaskDelete(NULL);
}
