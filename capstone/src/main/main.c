#include "config.h"
#include <task.h>
#include <string.h>
#include <queue.h>

#include "chess.h"
#include "game.h"
#include "clock.h"
#include "led.h"
#include "projdefs.h"
#include "sensor.h"
#include "uart_bidir_protocol.h"
#include "button.h"

#define DECLARE_PRIVATE_MAIN_C
#include "main.h"

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
    if (mainQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

BaseType_t xMain_sensor_update(BoardState *state) {
    MainThread_Message m;
    m.type = main_sensor_update;
    memcpy(&m.state, state, sizeof(BoardState));
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

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
                // Display the hint, clearing other things.
                xLED_save();
                xLED_clear_board();
                xIlluminateMove(state.hint_move, 0);
                xIlluminateMove(state.hint_move, 1);
                xLED_commit();
                state.hint = game_hint_displaying;
                break;
            case game_hint_displaying:
                // Turn off the hint, render whatever was shown before.
                state.hint = game_hint_known;
                xLED_restore();
                xLED_commit();
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
            xBoardEqual(&state.last_move_state, &state.last_measured_state) ==
                pdTRUE) {
            prvSwitchStateTurn(&state);
        } else {
            state.turn = game_turn_white;
        }
        state.state = game_state_notstarted;
        // Set the latest state to the board state.
        memcpy(&state.last_move_state, &state.last_measured_state,
               sizeof(BoardState));
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
            break;
        default:
            break;
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
                       prvPossibleMoves, prvPossibleMovesLen,
                       (state.turn == game_turn_white) ? pdTRUE : pdFALSE);
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
    memcpy(&state.last_move_state, &state.last_measured_state,
           sizeof(BoardState));
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

void prvRenderState() {
    if (state.hint == game_hint_displaying) {
        // Don't override the hint if we're currently displaying it.
        return;
    }
    if (prvPossibleMovesLen == 0) {
        // If we don't know all the possible moves, don't render any.
        return;
    }
    uint8_t row, col;
    BaseType_t xReturned = pdTRUE;
    // If the board state is unchanged, show the moveable pieces.
    if (xBoardEqual(&state.last_move_state, &state.last_measured_state) ==
        pdTRUE) {
        xReturned &= xLED_clear_board();
        xReturned &= xIlluminateMovable(prvPossibleMoves, prvPossibleMovesLen);
        xReturned &= xLED_commit();
        while (xReturned != pdTRUE) {
        }
    }
    // If it's not, check if we've only removed one piece.
    else if (xFindSingleLifted(&state.last_move_state,
                               &state.last_measured_state, &row,
                               &col) == pdTRUE) {
        // Okay, light up all the moves for that peice.
        xReturned &= xLED_clear_board();
        xReturned &= xIlluminatePieceMoves(prvPossibleMoves,
                                           prvPossibleMovesLen, row, col);
        xReturned &= xLED_commit();
        while (xReturned != pdTRUE) {
        }
    }
    // Otherwise, leave it unchanged.
}

void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case main_sensor_update:
        memcpy(&state.last_measured_state, &(message->state),
               sizeof(BoardState));
        prvRenderState();
        break;
    case main_uart_message:
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
            if (state.hint == game_hint_awaiting) {
                state.hint_move = message->move;
                state.hint = game_hint_displaying;
                // Save the current board display state.
                xLED_save();
                xLED_clear_board();
                xIlluminateMove(state.hint_move, 0);
                xIlluminateMove(state.hint_move, 1);
                xLED_commit();
            }
        }
        break;
    case main_button_press:
        prvHandleButtonPress(message->button);
        break;
    }
}