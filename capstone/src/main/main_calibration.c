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

We will use the turn to encode the sampling state. white is running, black is
paused.
*/
uint16_t numbers[2] = {0};

void mainThread(void *arg0) {
    MainThread_Message message;
    BaseType_t xReturned;

    xReturned = xMain_Init();
    while (xReturned != pdPASS) {}

    xReturned = xUART_Init();
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vUART_Task, "UART", configMINIMAL_STACK_SIZE, NULL,
                            3, &thread_uart);
    while (xReturned != pdPASS) {}

    // Wait until we get the proper synack packet from the RPi.
    // If we've waited more than HEARTBEAT_TIME_MS, try again.
    while (xQueueReceive(mainQueue, &message,
                         HEARTBEAT_TIME_MS / portTICK_PERIOD_MS) == pdFAIL ||
           message.move != SYNACK) {
        // Send our syn packet.
        xReturned = xUART_to_wire(MSP_SYN);
        while (xReturned != pdPASS);
    }
    // Okay, we've gotten the right synack packet. Cool.
    // Send the ack after we've initialized all our hardware and threads.

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
    xReturned = xTaskCreate(vSensor_Thread_Calibration, "Sensor",
                            configMINIMAL_STACK_SIZE, NULL, 2, &thread_sensor);
    while (xReturned != pdPASS) {}

    xClock_set_numbers(numbers);
    xClock_set_state(clock_state_staticnumbers);

    xReturned = xUART_to_wire(MSP_ACK);
    while (xReturned != pdPASS);

    volatile BaseType_t mem = xPortGetFreeHeapSize();
    while (1) {
        if (xQueueReceive(mainQueue, &message, portMAX_DELAY) == pdTRUE) {
            prvProcessMessage(&message);
        }
    }

    vTaskDelete(NULL);
}

BaseType_t xMain_Init(void) {
    max = 0;
    min = 65535;
    mainQueue = xQueueCreate(QUEUE_SIZE, sizeof(MainThread_Message));
    state.turn = game_turn_black;
    if (mainQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

BaseType_t xMain_sensor_update(BoardState *state) { return pdTRUE; }

BaseType_t xMain_button_press_FromISR(enum button_num button,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    MainThread_Message m;
    m.type = main_button_press;
    m.button = button;
    return xQueueSendFromISR(mainQueue, &m, pxHigherPriorityTaskWoken);
}

BaseType_t xMain_uart_message(uint32_t move) {
    MainThread_Message m;
    m.type = main_uart_message;
    m.move = move;
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

BaseType_t xMain_uart_message_FromISR(uint32_t move,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    MainThread_Message m;
    m.type = main_uart_message;
    m.move = move;
    return xQueueSendFromISR(mainQueue, &m, pxHigherPriorityTaskWoken);
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
            vUART_SendCalibration(min, max, row, col, selected_piece);
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
    xClock_set_state(clock_state_off);
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
        prvRenderState();
        if (state.turn == game_turn_black) {
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