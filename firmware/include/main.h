/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * main.h: definition of the interface for the main thread, as well as
 * some private structures and variables that made main.c clunky to work with.
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

#ifndef CAPSTONE_MAIN_H
#define CAPSTONE_MAIN_H

#include "game.h"
#include "button.h"
#include "semphr.h"

BaseType_t xMain_sensor_update(BoardState *state);
BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state);
BaseType_t xMain_button_press_FromISR(enum button_num button,
                                      BaseType_t *pxHigherPriorityTaskWoken);
BaseType_t xMain_uart_message(uint32_t move);
BaseType_t xMain_uart_message_FromISR(uint32_t move,
                                      BaseType_t *pxHigherPriorityTaskWoken);
BaseType_t xMain_time_up(void);

#ifdef DECLARE_PRIVATE_MAIN_C
// This is just for main.c, as a convenience. These are not part of the API.

enum MainThread_MsgType {
    main_sensor_update,
    main_button_press,
    main_uart_message,
    main_clock_timeover,
    main_quit // Only for the simulator, to make shutdown smooth.
};

typedef struct {
    enum MainThread_MsgType type;
    union {
#ifdef CALIBRATION
        BoardState_Calibration state;
#else
        BoardState state;
#endif
        enum button_num button;
        // Used for undo, hint, and possible moves.
        uint32_t move;
    };
} MainThread_Message;

#define MAX_POSSIBLE_MOVES (256)
static union {
    NormalMove possible[MAX_POSSIBLE_MOVES];
    UndoMove undo[MAX_POSSIBLE_MOVES];
} prvMoves;
static uint16_t prvMovesLen = 0;
static uint16_t prvCurrentMoveIndex = 0;
static GameState state;
#ifdef CALIBRATION
static PieceType selected_piece = WhitePawn;
static uint16_t max;
static uint16_t min;
#endif
// IDK, maybe change this later. Profiling?
#define QUEUE_SIZE 10
static QueueHandle_t mainQueue;

static TaskHandle_t thread_clock, thread_led, thread_sensor, thread_uart,
    thread_flashsquare;

BaseType_t xMain_Init(void);
static void prvSwitchTurnRoutine(void);
static void prvSwitchTurnUndo(void);
static void prvSwitchStateTurn(GameState *statevar);
static void prvHandleButtonPress(enum button_num button);
static void prvProcessMessage(MainThread_Message *message);
static void prvRenderState(void);
#endif

#endif
