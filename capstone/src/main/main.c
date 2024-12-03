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
#include "button.h"

#define DECLARE_PRIVATE_MAIN_C
#include "main.h"

void mainThread(void *arg0) {
    /* FIXME: declare task handle for UART task */
    BaseType_t xReturned;

    xReturned = xMain_Init();
    while (xReturned != pdPASS) {
    }

    // Heartbeat startup wait occurs here?
    xReturned = xUART_init();
    while (xReturned != pdPASS) {
    }

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
    vButton_Init();

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
    xReturned = xTaskCreate(vUART_Task, "UART", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_uart);
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

static void prvHandleButtonPress(enum button_num button) {
    BaseType_t xReturned;
    // If it's the turn-switch button:
    switch (button) {
    case button_num_white_move:
    case button_num_black_move:
        if ((state.turn == game_turn_black &&
             button == button_num_black_move) ||
            (state.turn == game_turn_white &&
             button == button_num_white_move)) {
            switch (state.state) {
            case game_state_notstarted:
            case game_state_running:
                prvSwitchTurnRoutine();
                break;
            case game_state_undo:
                prvSwitchTurnUndo();
                break;
            default:
                break;
            }
        }
        break;
    case button_num_pause:
        switch (state.state) {
        case game_state_running:
            // only if clock mode supports pausing
            if (state.clock_mode == game_clock_off) {
                break;
            }
            // TODO display some sort of pause message
            xClock_set_state(clock_state_paused);
            break;
        case game_state_paused:
            xClock_set_state(clock_state_running);
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
                // Request a hint from the pi.
                xReturned = xUART_EncodeEvent(BUTTON_HINT, 0);
                while (xReturned != pdPASS);
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
                // If we're awaiting it, just keep awaiting.
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
            // Check that clock mode is correct
            // I think this is right?
            if (state.clock_mode != game_clock_off) {
                break;
            }
            // TODO display something?
            prvSwitchStateTurn(&state);
            state.state = game_state_undo;
            prvMovesLen = 0;
            prvCurrentMoveIndex = 0;
            state.hint = game_hint_unknown;
        case game_state_undo:
            xReturned = xUART_EncodeEvent(BUTTON_UNDO, 0);
            while (xReturned != pdPASS);
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
    if (prvMovesLen == 0) {
        return;
    }
    // First, validate the most recent state.
    int16_t index = sFindMoveIndex(
        &state.last_move_state, &state.last_measured_state, prvMoves.possible,
        prvMovesLen, (state.turn == game_turn_white) ? pdTRUE : pdFALSE);
    if (index == -1) {
        // TODO: flash red.
        return;
    }

    // if it passes, proceed with switching turns.
    // Reset the possible moves.
    prvMovesLen = 0;
    prvCurrentMoveIndex = 0;
    BaseType_t xReturned =
        xUART_EncodeEvent(BUTTON_TURNSWITCH, prvMoves.possible[index]);
    while (xReturned != pdPASS);
    // Switch the player turn.
    prvSwitchStateTurn(&state);
    // Reset the hint state.
    state.hint = game_hint_unknown;
    // Set the board state to the latest measured state.
    memcpy(&state.last_move_state, &state.last_measured_state,
           sizeof(BoardState));
    // Clear the LEDs.
    xReturned = xLED_clear_board();
    while (xReturned != pdPASS);
    xReturned = xLED_commit();
    while (xReturned != pdPASS);

    // Switch the clock turn.
    xReturned = xClock_set_turn(state.turn);
    while (xReturned != pdPASS);
    // If we're just starting, start the clock.
    if (state.state == game_state_notstarted) {
        xReturned = xClock_set_state(clock_state_running);
        while (xReturned != pdPASS);
        state.state = game_state_running;
    }
}

static void prvSwitchTurnUndo(void) {
    BaseType_t xReturned;
    // If we're still waiting for the just-requested undo, ignore the request.
    if (prvMovesLen == 0) {
        return;
    }

    // Validate the most recent state.
    if (xCheckUndo(&state.last_move_state, &state.last_measured_state,
                   prvMoves.undo[0],
                   (state.turn == game_turn_white) ? pdTRUE : pdFALSE) !=
        pdTRUE) {
        // Validation failed.
        // TODO: flash red.
        return;
    }

    // Dequeue and shift the completed undo move.
    prvMovesLen--;
    for (uint16_t i = 0; i < prvMovesLen; i++) {
        prvMoves.undo[i] = prvMoves.undo[i + 1];
    }
    // Update the last valid board state.
    memcpy(&state.last_move_state, &state.last_measured_state,
           sizeof(BoardState));

    xReturned = xLED_clear_board();
    while (xReturned != pdPASS);
    xReturned = xLED_commit();
    while (xReturned != pdPASS);

    // Check: are we done with undos?
    if (prvMovesLen == 0) {
        // Switch back to normal mode.
        // Do not switch the player turn.
        state.state = game_state_running;
        state.hint = game_hint_unknown;
        prvCurrentMoveIndex = 0;
        // Send dummy packet to uart to request moves.
        xReturned = xUART_to_wire(0);
        while (xReturned != pdPASS);
        // TODO clock?
        xReturned = xClock_set_turn(state.turn == game_turn_black);
        while (xReturned != pdPASS);
    } else {
        prvSwitchStateTurn(&state);
    }
}

static void prvRenderState(void) {
    if (state.hint == game_hint_displaying) {
        // Don't override the hint if we're currently displaying it.
        return;
    }
    if (prvMovesLen == 0) {
        // If we don't know all the possible moves, don't render any.
        // Alternatively, if undoing: if we don't have an undo move, don't show
        // anything.
        return;
    }
    uint8_t row, col;
    BaseType_t xReturned = pdTRUE;
    if (state.state == game_state_undo) {
        // Render the undo move. If the dest square contents have
        // changed, we are good to clobber.
        row = GET_SRC_RANK(prvMoves.undo[0]);
        col = GET_SRC_FILE(prvMoves.undo[0]);
        BaseType_t clobber = pdFALSE;
        if (xGetSquare(&state.last_move_state, row, col) !=
            xGetSquare(&state.last_measured_state, row, col)) {
            clobber = pdTRUE;
        }
        xReturned &= xLED_clear_board();
        xReturned &= xIlluminateUndo(prvMoves.undo[0], clobber);
        xReturned &= xLED_commit();
        while (xReturned != pdTRUE) {
        }
        return;
    }
    // If the board state is unchanged, show the moveable pieces.
    if (xBoardEqual(&state.last_move_state, &state.last_measured_state) ==
        pdTRUE) {
        xReturned &= xLED_clear_board();
        xReturned &= xIlluminateMovable(prvMoves.possible, prvMovesLen);
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
        xReturned &=
            xIlluminatePieceMoves(prvMoves.possible, prvMovesLen, row, col);
        xReturned &= xLED_commit();
        while (xReturned != pdTRUE) {
        }
    }
    // Otherwise, leave it unchanged.
}

static void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case main_sensor_update:
        memcpy(&state.last_measured_state, &(message->state),
               sizeof(BoardState));
        prvRenderState();
        break;
    case main_uart_message:
        // If we're in an undo state, append to the undo queue.
        if (state.state == game_state_undo) {
            // We can't accept any more undo moves if we're over the limit.
            if (prvMovesLen < 256) {
                prvMoves.undo[prvMovesLen] = message->move;
                prvMovesLen++;
            }
        }
        // Check: are we currently listening for moves?
        else if (prvMovesLen == 0) {
            prvMoves.possible[prvCurrentMoveIndex] = message->move;
            prvCurrentMoveIndex++;
            if (IS_LAST_MOVE(message->move)) {
                prvMovesLen = prvCurrentMoveIndex;
                // Movable pieces will be rendered on the next sensor input.
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
