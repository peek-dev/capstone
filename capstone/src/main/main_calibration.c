#include "config.h"
#include <task.h>
#include <string.h>
#include <queue.h>

#include "chess.h"
#include "game.h"
#include "clock.h"
#include "led.h"
#include "portmacro.h"
#include "projdefs.h"
#include "sensor.h"
#include "uart.h"
#include "uart_bidir_protocol.h"
#include "led_translation.h"
#include "button.h"

#define CALIBRATION
#define DECLARE_PRIVATE_MAIN_C
#include "main.h"

/*
Calibration testbench.
We will use the state somewhat differently here.
There are four pieces of state we care about:
 - selected square (0-64)
 - selected ptype
 - sampling state (running or paused)
 - measured value limits (min, max)
Because we need raw ADC values, we will use a 
modified version of the sensor thread.

We will use the turn to encode the sampling state. white is running, black is paused.
*/
uint16_t numbers[2] = {0};

void mainThread(void *arg0) {
    /* FIXME: declare task handle for UART task */
    BaseType_t xReturned;

    xReturned = xMain_Init();
    while (xReturned != pdPASS) {}

    // Heartbeat startup wait occurs here?
    xReturned = xUART_init();
    while (xReturned != pdPASS) {}

    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {}
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {}
    xReturned = xSensor_Init();
    while (xReturned != pdPASS) {}
    vButton_Init();

    xReturned = xPortGetFreeHeapSize();

    // Initialize the system threads.
    xReturned = xTaskCreate(vClock_Thread, "Clock", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_clock);
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vLED_Thread, "LED", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_led);
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vSensor_Thread_Calibration, "Sensor", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_sensor);
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vUART_Task, "UART", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_uart);
    while (xReturned != pdPASS) {}

    xClock_set_numbers(numbers);
    xClock_set_state(clock_state_staticnumbers);

    MainThread_Message message;
    volatile BaseType_t mem = xPortGetFreeHeapSize();
    while (1) {
        if (xQueueReceive(mainQueue, &message, portMAX_DELAY) == pdTRUE) {
            prvProcessMessage(&message);
        }
    }

    vTaskDelete(NULL);
}

BaseType_t xMain_Init(void) {
    mainQueue = xQueueCreate(QUEUE_SIZE, sizeof(MainThread_Message));
    state.turn = game_turn_black;
    if (mainQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

BaseType_t xMain_sensor_update(BoardState *state) { return pdTRUE; }

BaseType_t xMain_button_press(enum button_num button) {
    MainThread_Message m;
    m.type = main_button_press;
    m.button = button;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t retval =
        xQueueSendFromISR(mainQueue, &m, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return retval;
}

BaseType_t xMain_uart_message(uint32_t move) {
    MainThread_Message m;
    m.type = main_uart_message;
    m.move = move;
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state) {
    MainThread_Message m;
    m.type = main_sensor_update;
    memcpy(&m.state, state, sizeof(BoardState_Calibration));
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

static void prvSwitchStateTurn(GameState *statevar) {
    if (statevar->turn == game_turn_black) {
        statevar->turn = game_turn_white;
    } else {
        statevar->turn = game_turn_black;
    }
}

static void prvHandleButtonPress(enum button_num button) {
    uint8_t row = prvMovesLen / 8;
    uint8_t col = prvMovesLen % 8;
    // If it's the turn-switch button:
    switch (button) {
    case button_num_white_move:
        prvMovesLen = (prvMovesLen + 63) % 64;
        break;
    case button_num_black_move:
        prvMovesLen = (prvMovesLen + 1) % 64;
        break;
    case button_num_start_restart:
        selected_piece = (selected_piece + 1) % (BlackPawn + 1);
        break;
    case button_num_clock_mode:
        selected_piece = (selected_piece + BlackPawn) % (BlackPawn + 1);
        break;
    case button_num_undo:
        if (state.turn == game_turn_white) {
            // If we just finished a calibration stage, send it to the UART.
            xUART_SendCalibration(min, max, row, col, selected_piece);
            // reset min and max.
            min = 65535;
            max = 0;
        }
        prvSwitchStateTurn(&state);
        // Skip state rerender.
        return;
    default:
        break;
    }
    prvRenderState();
}


static void prvRenderState(void) {
    uint8_t row = prvMovesLen / 8;
    uint8_t col = prvMovesLen % 8;
    Color color = {.brightness = 31, .red = 255, .green = 255, .blue = 255};
    // Illuminate the square and the piece type on that square.
    xLED_clear_board();
    xLED_set_color(LEDTrans_Square(row, col), &color);
    ZeroToTwoInts pieceOutlines = LEDTrans_Ptype(selected_piece);
    for (uint8_t i = 0; i < pieceOutlines.len; i++) {
        xLED_set_color(pieceOutlines.data[i], &color);
    }
    xLED_commit();
}

static void prvProcessMessage(MainThread_Message *message) {
    uint8_t row = prvMovesLen / 8;
    uint8_t col = prvMovesLen % 8;
    uint16_t oldmax, oldmin;
    uint16_t numbers[2];
    uint16_t value;
    switch (message->type) {
    case main_sensor_update:
        if (state.turn == game_turn_black) {
            xClock_set_state(clock_state_off);
            break;
        }
        value = message->state.rows[row].columns[col];
        oldmax = max;
        oldmin = min;
        if (value > max) { 
            max = value;
        }
        if (value < min) {
            min = value;
        }
        if (oldmax != max || oldmin != min) {
            numbers[0] = max;
            numbers[1] = min;
            xClock_set_numbers(numbers);
            xClock_set_state(clock_state_staticnumbers);
        }
        break;
    case main_uart_message:
        break;
    case main_button_press:
        prvHandleButtonPress(message->button);
        break;
    }
}