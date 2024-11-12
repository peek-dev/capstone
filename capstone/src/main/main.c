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
    mainQueue = xQueueCreate(QUEUE_SIZE, sizeof(Clock_Message));

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

    MainThread_Message message;
    while

    vTaskDelete(NULL);
}
static void prvSwitchTurnRoutine();

static void prvSwitchStateTurn(GameState *statevar) {
    if (statevar->turn == game_turn_black) {
        statevar->turn = game_turn_white;
    } else {
        statevar->turn = game_turn_black;
    }
}
void prvHandleButtonPress(enum button_num button) {
    // If it's the turn-switch button:
    switch (button) {
    case button_num_white_move:
    case button_num_black_move:
        if (state.turn == game_turn_black && button == button_num_black_move ||
            state.turn == game_turn_white && button == button_num_white_move) {
            switch (state.state) {
            case game_state_notstarted:
            case game_state_running:
                prvSwitchTurnRoutine();
                break;
            case game_state_undo:
                // TODO undos
                break;
            default:
                break;
            }
        }
        break;
    case button_num_pause:
        switch (state.state) {
        case game_state_running:
            // TODO display some sort of pause message
            // TODO only if clock mode supports pausing
            xClock_stop_clock();
            break;
        case game_state_paused:
            // TODO only if clock mode supports pausing
            xClock_start_clock();
            break;
        default:
            break;
        }
        break;
    case button_num_hint:
        switch (state.state) {
            case game_state_running:
            case game_state_paused:
            case game_state_notstarted:
                switch (state.hint) {
                    case game_hint_unknown:
                        // TODO request hint from uart;
                        state.hint = game_hint_awaiting;
                        break;
                    case game_hint_known:
                        // TODO display hint
                        state.hint = game_hint_displaying;
                        break;
                    case game_hint_displaying:
                        // TODO display movable pieces
                        state.hint = game_hint_known;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;

        }
        break;
    case button_num_start_restart:
        state.hint = game_hint_unknown;
        if (state.state == game_state_notstarted && 
            xBoardEqual(&state.last_move_state, &state.last_measured_state) == pdTRUE) {
            prvSwitchStateTurn(&state);
        } else {
            state.turn = game_turn_white;
        }
        state.state = game_state_notstarted;
        // Set the latest state to the board state.
        memcpy(&state.last_move_state, &state.last_measured_state, sizeof(BoardState));
        // TODO send the state to the uart somehow.
        break;
    case button_num_clock_mode:
        if (state.state == game_state_notstarted) {
            // TODO cycle through clock modes.
        }
        break;
    case button_num_undo:
        switch (state.state) {
            case game_state_paused:
            case game_state_running:
                // TODO request an undo move from the uart.
                // TODO finish this
        }
        break;
    }
}


// Should only be called when in the running and reset states.
static void prvSwitchTurnRoutine() {
    // If we haven't finished getting possible moves yet, ignore the request.
    if (prvPossibleMovesLen == 0) {
        return;
    }
    // First, validate the most recent state.
    int16_t index =
        sFindMoveIndex(&state.last_move_state, &state.last_measured_state,
                       prvPossibleMoves, prvPossibleMovesLen);
    if (index == -1) {
        // TODO: flash red.
        return;
    }

    // if it passes, proceed with switching turns.
    // Reset the possible moves.
    prvPossibleMovesLen = 0;
    prvCurrentMoveIndex = 0;
    // TODO send a message back to the UART
    // Switch the player turn.
    prvSwitchStateTurn(&state);
    // Reset the hint state.
    state.hint = game_hint_unknown;
    // Set the board state to the latest measured state.
    memcpy(&state.last_move_state, &state.last_measured_state, sizeof(BoardState));
    // Clear the LEDs.
    xLED_clear_board();
    xLED_commit();
    
    // Switch the clock turn.
    xClock_set_turn(state.turn == game_turn_black);
    // If we're just starting, start the clock.
    if (state.state == game_state_notstarted) {
        xClock_start_clock();
        state.state = game_state_running;
    }
}

void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case chess_sensor_update:
        memcpy(&state.last_measured_state, &(message->state),
               sizeof(BoardState));
        // TODO: detect piece lift, respond.
        break;
    case chess_uart_move:
        // Check: are we currently listening for moves?
        if (prvPossibleMovesLen == 0) {
            prvPossibleMoves[prvCurrentMoveIndex] = message->move;
            prvCurrentMoveIndex++;
            if (IS_LAST_MOVE(message->move)) {
                prvPossibleMovesLen = prvCurrentMoveIndex;
                // TODO: use LEDs to show which pieces can move.
            }
        } else {
            // This must be a hint move.
            state.hint_move = message->move;
            if (state.hint == game_hint_awaiting) {
                state.hint = game_hint_displaying;
                // TODO: display the hint.
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