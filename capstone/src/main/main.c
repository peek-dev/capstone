#include "config.h"
#include <task.h>
#include "string.h"

#include "chess.h"
#include "game.h"
#include "clock.h"
#include "led.h"
#include "sensor.h"
#include "uart_bidir_protocol.h"
#include "button.h"

#define MAX_POSSIBLE_MOVES 256
static NormalMove prvPossibleMoves[MAX_POSSIBLE_MOVES];
static uint8_t prvPossibleMovesLen = 0;
static uint8_t prvCurrentMoveIndex = 0;
static GameState state;

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
        enum button_num button;
        NormalMove move;
    };
} MainThread_Message;

void mainThread(void *arg0) {
    TaskHandle_t thread_clock, thread_led, thread_sensor;
    BaseType_t xReturned;

    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {
    }
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {
    }
    xReturned = xSensor_Init();
    while (xReturned != pdPASS) {
    }

    xReturned = xPortGetFreeHeapSize();

    // Initialize the system threads.
    xReturned = xTaskCreate(vClock_Thread, "Clock", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_clock);
    while (xReturned != pdPASS) {
    }
    xReturned = xTaskCreate(vLED_Thread, "LED", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_led);
    while (xReturned != pdPASS) {
    }
    xReturned = xTaskCreate(vSensor_Thread, "Sensor", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_sensor);
    while (xReturned != pdPASS) {
    }
    xReturned = xPortGetFreeHeapSize();

    xReturned = xClock_run_test(1);
    while (xReturned != pdPASS) {
    }
    vTaskDelete(NULL);
}
static void prvSwitchTurnRoutine();

void prvHandleButtonPress(enum button_num button) {
    // If it's the turn-switch button:
    switch (button) {
    case button_num_white_move:
    case button_num_black_move:
        if (state.turn == game_turn_black && button == button_num_black_move ||
            state.turn == game_turn_white && button == button_num_white_move) {
            switch (state.state) {
            case game_state_running:
                prvSwitchTurnRoutine();
                break;
            case game_state_undo:
                // TODO undos
                break;
            case game_state_notstarted:
                xClock_start_clock();
                prvSwitchTurnRoutine();
                break;
            default:
                break;
            }
        }
        break;
    case button_num_pause:
        switch (state.state) {
        case game_state_running:
            xClock_stop_clock();
            break;
        case game_state_paused:
            xClock_start_clock();
            break;
        default:
            break;
        }
        break;
    case button_num_hint:
        break;
    case button_num_start_restart:
        break;
    case button_num_clock_mode:
        break;
    case button_num_undo:
        break;
    }
}

static void prvSwitchTurnRoutine() {
    // First, validate the most recent state.
    int16_t index =
        sFindMoveIndex(&state.last_move_state, &state.last_measured_state,
                       prvPossibleMoves, prvPossibleMovesLen);
    if (index == -1) {
        // TODO: flash red.
        return;
    }
    // if it passes, proceed with switching turns.
    // TODO send a message back to the UART

    // Switch the clock turn.
    xClock_switch_turn();
}

void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case chess_sensor_update:
        memcpy(&state.last_measured_state, &(message->state),
               sizeof(BoardState));
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
        prvHandleButtonPress(message->button);
        break;
    case chess_uart_undo:
        // TODO
        break;
    }
}