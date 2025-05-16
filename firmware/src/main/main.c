/**
 * Copyright (C) 2024 John E. Berberian, Jr., and Paul D. Karhnak
 *
 * main.c: the core event loop for the MSPM0G3507 to initialize
 * C.H.E.S.S.B.O.A.R.D. firmware state and begin communicating with the
 * Raspberry Pi to "wake up" the full scope of the C.H.E.S.S.B.O.A.R.D.
 * computer systems' capabilities.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */
#include "config.h"
#include <task.h>
#include <string.h>
#include <queue.h>

#include "chess.h"
#include "flash_square.h"
#include "game.h"
#include "clock.h"
#include "led.h"
#include "led_translation.h"
#include "portmacro.h"
#include "projdefs.h"
#include "sensor.h"
#include "uart.h"
#include "uart_bidir_protocol.h"
#include "button.h"
#include "sensor_mutex.h"

#define DECLARE_PRIVATE_MAIN_C
#include "main.h"

static BaseType_t finished_handshake = pdFALSE;
BaseType_t rendered_after_hint = pdFALSE;
BaseType_t correct_after_reset = pdTRUE;

static void vResetState();

void mainThread(void *arg0) {
    (void)arg0;
    sensor_mutex = xSemaphoreCreateMutex();
    MainThread_Message message;
    BaseType_t xReturned;

    // Make the display consistent while we initialize.
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {}
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {}
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
    xReturned = xSensor_Init();
    while (xReturned != pdPASS) {}
    xReturned = xFlashSquare_Init();
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
    xReturned = xTaskCreate(vSensor_Thread, "Sensor", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_sensor);
    while (xReturned != pdPASS) {}
    // Much higher priority so that it gets scheduling precedence.
    xReturned =
        xTaskCreate(vFlashSquare_Thread, "FlashSquare",
                    configMINIMAL_STACK_SIZE, NULL, 5, &thread_flashsquare);
    while (xReturned != pdPASS) {}

    xReturned = xUART_to_wire(MSP_ACK);
    while (xReturned != pdPASS);
    finished_handshake = pdTRUE;
    vResetState();
    xClock_render_state();

    MAKEVISIBLE BaseType_t mem = xPortGetFreeHeapSize();
    while (1) {
        if (xQueueReceive(mainQueue, &message, portMAX_DELAY) == pdTRUE) {
            if (message.type == main_quit) {
                break;
            }
            prvProcessMessage(&message);
        }
    }

    vTaskDelete(NULL);
}

static void vSetClockState() {
    BaseType_t xReturned = pdTRUE;
    BaseType_t prep_clock = pdTRUE;
    uint32_t inc = 0;
    uint32_t times[2] = {0};
    switch (state.clock_mode) {
    case game_clock_off:
        xReturned &= xClock_set_state(clock_state_off);
        prep_clock = pdFALSE;
        break;
    case game_clock_90_plus30:
        times[0] = 90 * 60 * 1000;
        inc = 30 * 1000;
        break;
    case game_clock_25_plus10:
        times[0] = 25 * 60 * 1000;
        inc = 10 * 1000;
        break;
    case game_clock_5_plus3:
        times[0] = 5 * 60 * 1000;
        inc = 3 * 1000;
        break;
    }
    if (prep_clock == pdTRUE) {
        times[1] = times[0];
        xReturned &= xClock_set_times(times);
        // Setting inc forces a rerender, which is needed when switching
        // between the timing modes. So the order unfortunately matters.
        xReturned &= xClock_set_increment(inc);
        xReturned &= xClock_set_state(clock_state_notstarted);
    }
    while (xReturned != pdPASS);
}

static void vResetState() {
    state.turn = game_turn_white;
    state.clock_mode = game_clock_off;
    state.state = game_state_notstarted;
    state.hint = game_hint_unknown;
    state.in_check = pdFALSE;
    state.awaiting_undo = pdFALSE;
    prvMovesLen = 0;
    prvCurrentMoveIndex = 0;
    vBoardSetDefault(&state.last_move_state);
    correct_after_reset = pdFALSE;
    if (finished_handshake == pdTRUE) {
        vSetClockState();
    }
    xLED_clear_board();
    xLED_save(1);
    rendered_after_hint = pdTRUE;
    xLED_commit();
}

SemaphoreHandle_t sensor_mutex;
BaseType_t xMain_Init(void) {
    mainQueue = xQueueCreate(QUEUE_SIZE, sizeof(MainThread_Message));
    vResetState();
    if (mainQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

BaseType_t xMain_time_up(void) {
    MainThread_Message m;
    m.type = main_clock_timeover;
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

BaseType_t xMain_sensor_update(BoardState *state) {
    MainThread_Message m;
    m.type = main_sensor_update;
    memcpy(&m.state, state, sizeof(BoardState));
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

BaseType_t xMain_button_press_FromISR(enum button_num button,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    MainThread_Message m;
    m.type = main_button_press;
    m.button = button;
    return xQueueSendFromISR(mainQueue, &m, pxHigherPriorityTaskWoken);
}

BaseType_t prvMain_button_press(enum button_num button) {
    MainThread_Message m;
    m.type = main_button_press;
    m.button = button;
    // This queue can only be dequeued by this thread, so we would
    // prefer to just drop the packet if it can't fit.
    return xQueueSend(mainQueue, &m, 0);
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
            xClock_set_state(clock_state_paused);
            state.state = game_state_paused;
            break;
        case game_state_paused:
            xClock_set_state(clock_state_running);
            state.state = game_state_running;
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
                if (prvMovesLen != 0 &&
                    xBoardEqual(&state.last_move_state,
                                &state.last_measured_state)) {
                    // Request a hint from the pi.
                    xReturned = xUART_EncodeEvent(BUTTON_HINT, 0);
                    while (xReturned != pdPASS);
                    state.hint = game_hint_awaiting;
                }
                break;
            case game_hint_known:
                if (xBoardEqual(&state.last_move_state,
                                &state.last_measured_state)) {
                    // Display the hint, clearing other things.
                    xLED_clear_board();
                    xIlluminateMove(state.hint_move, 0);
                    xIlluminateMove(state.hint_move, 1);
                    xLED_commit();
                    state.hint = game_hint_displaying;
                }
                break;
            case game_hint_displaying:
                // Turn off the hint, render whatever was shown before.
                state.hint = game_hint_known;
                xLED_restore(0);
                xLED_commit();
                rendered_after_hint = pdFALSE;
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
        if (state.state == game_state_notstarted &&
            xBoardEqual(&state.last_move_state, &state.last_measured_state) ==
                pdTRUE) {
            // prvSwitchStateTurn(&state);
            vResetState();
        } else {
            vResetState();
        }
        xUART_EncodeEvent(BUTTON_RESTART, 0);

        break;
    case button_num_clock_mode:
        if (state.state == game_state_notstarted) {
            state.clock_mode =
                (state.clock_mode + 1) % (game_clock_5_plus3 + 1);
            vSetClockState();
        }
        break;
    case button_num_undo:
        if (state.state == game_state_undo && state.awaiting_undo == pdTRUE) {
            // If we're currently waiting for an undo message to be returned,
            // we can't handle another undo press right now. Sleep for 1ms and re-queue.
            // Hopefully, this will be enough time for the UART message to get sent
            // but not enough to introduce any perceptible delay.
            // Actually, no reason to cycle through all of the delays for each time.
            // Just re-queue it instantly.
            //vTaskDelay(pdMS_TO_TICKS(1));
            // To ensure there is sufficient space for the uart message, only re-queue if there is
            // a good bit of space left.
            if (uxQueueSpacesAvailable(mainQueue) > 4) {
                prvMain_button_press(button_num_undo);
            }
            break;
        }
        switch (state.state) {
        case game_state_paused:
        case game_state_running:
        case game_state_over:
            // Check that clock mode is correct
            // I think this is right?
            if (state.clock_mode != game_clock_off) {
                break;
            }
            prvSwitchStateTurn(&state);
            xClock_set_state(clock_state_undo);
            xClock_set_turn(state.turn);
            state.state = game_state_undo;
            prvMovesLen = 0;
            prvCurrentMoveIndex = 0;
            state.hint = game_hint_unknown;
        // fall through
        case game_state_undo:
            if (prvMovesLen == MAX_POSSIBLE_MOVES) {
                // Ignore the button press if we're out of room
                break;
            }
            state.awaiting_undo = pdTRUE;
            xClock_set_both_numbers(prvMovesLen + 1);
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
    BaseType_t is_partial = pdFALSE;
    int16_t index = sFindMoveIndex(
        &state.last_move_state, &state.last_measured_state, prvMoves.possible,
        prvMovesLen, (state.turn == game_turn_white) ? pdTRUE : pdFALSE,
        &is_partial);
    if (index == -1) {
        vFlashDifferent(&state.last_move_state, &state.last_measured_state);
        return;
    }
    if (is_partial == pdTRUE) {
        xIlluminatePartial(prvMoves.possible[index],
                           (state.turn == game_turn_white) ? pdTRUE : pdFALSE);
        xLED_commit();
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
    state.in_check = pdFALSE;
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
        if (state.clock_mode != game_clock_off) {
            xReturned = xClock_set_state(clock_state_running);
            while (xReturned != pdPASS);
        }
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
        // vFlashDifferent(&state.last_move_state, &state.last_measured_state);
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
        state.in_check = pdFALSE;
        prvCurrentMoveIndex = 0;
        // Send dummy packet to uart to request moves.
        xReturned = xUART_to_wire(SENTINEL_REQUEST_RESEND_MOVES);
        while (xReturned != pdPASS);
        xReturned = xClock_set_turn(state.turn == game_turn_black);
        if (state.clock_mode != game_clock_off) {
            // dead code, I think?
            xReturned &= xClock_set_state(clock_state_running);
        } else {
            xReturned &= xClock_set_state(clock_state_off);
        }
        while (xReturned != pdPASS);
    } else {
        prvSwitchStateTurn(&state);
        xClock_set_turn(state.turn);
        xClock_set_both_numbers(prvMovesLen);
    }
}

static BaseType_t prvCheckSentinel(uint32_t packet) {
    if ((packet >> 16) == 0) {
        switch (packet) {
        case SYNACK:
            // Ignore extra synack packets.
            break;
        case SENTINEL_CHECKMATE:
            state.state = game_state_over;
            // Hehe this switches turn internally, see header definition.
            state.winner = state.turn;
            // cut off move listening.
            prvMovesLen = 1;
            xClock_set_turn(game_turn_over);
            break;
        case SENTINEL_STALEMATE:
            // Stalemate rendering
            state.state = game_state_over;
            state.winner = game_winner_draw;
            prvMovesLen = 1;
            xClock_set_turn(game_turn_over);
            break;
        default:
            state.check_col = GET_M2_DEST_FILE(packet);
            state.check_row = GET_M2_DEST_RANK(packet);
            state.in_check = pdTRUE;
            break;
        }
        return pdTRUE;
    }
    return pdFALSE;
}

static void prvRenderState(void) {
    if (prvMovesLen == 0) {
        // If we don't know all the possible moves, don't render any.
        // Alternatively, if undoing: if we don't have an undo move, don't show
        // anything.
        return;
    }
    // Light up the piece outlines to indicate the game being over.
    if (state.state == game_state_over) {
        Color blackColor;
        Color whiteColor;
        switch (state.winner) {
        case game_winner_black:
            blackColor = Color_Winner;
            whiteColor = Color_Loser;
            break;
        case game_winner_white:
            blackColor = Color_Loser;
            whiteColor = Color_Winner;
            break;
        case game_winner_draw:
            blackColor = Color_Draw;
            whiteColor = Color_Draw;
            break;
        }
        ZeroToTwoInts z;
        xLED_clear_board();
        for (PieceType p = WhitePawn; p < EmptySquare; p++) {
            z = LEDTrans_Ptype(p);
            for (uint8_t i = 0; i < z.len; i++) {
                xLED_set_color(z.data[i], &whiteColor);
            }
        }
        for (PieceType p = BlackKing; p <= BlackPawn; p++) {
            z = LEDTrans_Ptype(p);
            for (uint8_t i = 0; i < z.len; i++) {
                xLED_set_color(z.data[i], &blackColor);
            }
        }
        xLED_commit();
        return;
    }
    uint8_t row = 12, col = 12;
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
        while (xReturned != pdTRUE) {}
        return;
    }
    BaseType_t board_changed = pdTRUE;
    // If the board state is unchanged, show the moveable pieces.
    if (xBoardEqual(&state.last_move_state, &state.last_measured_state) ==
        pdTRUE) {
        if (correct_after_reset == pdFALSE) {
            correct_after_reset = pdTRUE;
        }
        xReturned &= xFlashSquare_DisableAll();
        xReturned &= xLED_clear_board();
        xReturned &= xIlluminateMovable(prvMoves.possible, prvMovesLen);
        xLED_save(1);
        rendered_after_hint = pdTRUE;
    }
    // If it's not, check if we've only removed one piece.
    else if (xFindSingleLifted(&state.last_move_state,
                               &state.last_measured_state, &row,
                               &col) == pdTRUE) {

        xReturned &= xFlashSquare_DisableAll(); // Okay, light up all the moves
                                                // for that peice.
        xReturned &= xLED_clear_board();
        xReturned &=
            xIlluminatePieceMoves(prvMoves.possible, prvMovesLen, row, col);
        xLED_save(1);
        rendered_after_hint = pdTRUE;

    } else {
        board_changed = pdFALSE;
        if (state.hint != game_hint_displaying &&
            rendered_after_hint == pdTRUE) {
            xIlluminatePotentiallyOffCenter(&state.last_move_state,
                                            &state.last_measured_state,
                                            &board_changed);
        }
    }
    if (state.state == game_state_notstarted &&
        correct_after_reset == pdFALSE) {
        xLED_clear_board();
        vInvalidDifferent(&state.last_move_state, &state.last_measured_state);
        xLED_commit();
        return;
    }
    if (board_changed) {
        if (state.in_check == pdTRUE &&
            (row != state.check_row || col != state.check_col)) {
            xReturned &= xLED_set_color(
                LEDTrans_Square(state.check_row, state.check_col),
                &Color_Check);
        }
        if (state.hint == game_hint_displaying) {
            xReturned &= xLED_save(0);
            xReturned &= xLED_clear_board();
            xReturned &= xIlluminateMove(state.hint_move, 0);
            xReturned &= xIlluminateMove(state.hint_move, 1);
        }
        xReturned &= xLED_commit();
        while (xReturned != pdTRUE) {}
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
            if (message->move == SENTINEL_UNDO_EXHAUSTED) {
                //xClock_set_both_numbers(prvMovesLen);
                if (prvMovesLen == 0) {
                    state.state = game_state_notstarted;
                    vSetClockState();
                    // Pressing undo will have switched the turn, so this switches it back.
                    prvSwitchStateTurn(&state);
                    xUART_to_wire(SENTINEL_REQUEST_RESEND_MOVES);
                }
                break;
            }
            // We can't accept any more undo moves if we're over the limit.
            if (prvMovesLen < MAX_POSSIBLE_MOVES) {
                state.awaiting_undo = pdFALSE;
                prvMoves.undo[prvMovesLen] = message->move;
                prvMovesLen++;
            } else {
                // TODO send back the move to prevent inconsistent state?
            }
        }
        // Check: are we currently listening for moves?
        else if (prvMovesLen == 0) {
            if (prvCheckSentinel(message->move) == pdTRUE) {
                break;
            }
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
    case main_clock_timeover:
        prvCheckSentinel(SENTINEL_CHECKMATE);
        break;
    case main_quit:
        // should never happen - this should never make it to this point.
        return;
    }
}
BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state) {
    (void)state;
    return pdTRUE;
}
