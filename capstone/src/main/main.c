#include "config.h"
#include <task.h>

#include "chess.h"
#include "game.h"
#include "clock.h"
#include "led.h"
#include "sensor.h"
#include "uart_bidir_protocol.h"

#define MAX_POSSIBLE_MOVES 256
static NormalMove prvPossibleMoves[MAX_POSSIBLE_MOVES];
static uint8_t prvPossibleMovesLen = 0;
static uint8_t prvCurrentMoveIndex = 0;
static BoardState prvLastMoveState;
static BoardState prvMostRecentState;

enum MainThread_MsgType {
    chess_sensor_update,
    chess_button_press,
    chess_uart_move,
    chess_uart_undo
};

typedef struct {
    enum MainThread_MsgType type;
    union {
        // TODO: this is sooo inefficient. 32 bytes vs 4?
        BoardState state;
        // TODO: buttons, uart (separate undo and new move)
        uint8_t button_num;
        NormalMove move;
    };
} MainThread_Message;

void mainThread(void *arg0) {
    TaskHandle_t thread_clock, thread_led, thread_sensor;
    BaseType_t xReturned;

    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {}
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {}
    xReturned = xSensor_Init();
    while (xReturned != pdPASS) {}

    xReturned = xPortGetFreeHeapSize();

    // Initialize the system threads.
    xReturned = xTaskCreate(vClock_Thread, "Clock", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_clock);
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vLED_Thread, "LED", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_led);
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vSensor_Thread, "Sensor", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_sensor);
    while (xReturned != pdPASS) {}
    xReturned = xPortGetFreeHeapSize();

    xReturned = xClock_run_test(1);
    while (xReturned != pdPASS) {
    }
    vTaskDelete(NULL);
}

void prvHandleButtonPress(uint8_t button_num) {
    // If it's the turn-switch button:
    if (1) {
        // First, validate the most recent state.
        int16_t index = sFindMoveIndex(&prvLastMoveState, &prvMostRecentState, prvPossibleMoves, prvPossibleMovesLen + 1);
        if (index == -1) {
            return;
        }
        // if it passes, proceed with switching turns.
        // send a message back to the 
    }
}

void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
        case chess_sensor_update:
            memcpy(&prvMostRecentState, &(message->state), sizeof(BoardState));
            break;
        case chess_uart_move:
            if (prvPossibleMovesLen == 0) {
                prvPossibleMoves[prvCurrentMoveIndex] = message->move;
                prvCurrentMoveIndex++;
                if (IS_LAST_MOVE(message->move)) {
                    prvPossibleMovesLen = prvCurrentMoveIndex;
                }
            }
            break;
        case chess_button_press:
            prvHandleButtonPress(message->button_num);
            break;
        case chess_uart_undo:
            // TODO
            break;
    }
}