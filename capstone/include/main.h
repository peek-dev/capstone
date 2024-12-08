#ifndef CAPSTONE_MAIN_H
#define CAPSTONE_MAIN_H

#include "game.h"
#include "button.h"

BaseType_t xMain_sensor_update(BoardState *state);
BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state);
BaseType_t xMain_button_press_FromISR(enum button_num button,
                                      BaseType_t *pxHigherPriorityTaskWoken);
BaseType_t xMain_uart_message(uint32_t move);
BaseType_t xMain_uart_message_FromISR(uint32_t move,
                                      BaseType_t *pxHigherPriorityTaskWoken);

#ifdef DECLARE_PRIVATE_MAIN_C
// This is just for main.c, as a convenience. These are not part of the API.

enum MainThread_MsgType {
    main_sensor_update,
    main_button_press,
    main_uart_message
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

#define MAX_POSSIBLE_MOVES 256
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

static TaskHandle_t thread_clock, thread_led, thread_sensor, thread_uart;

BaseType_t xMain_Init(void);
static void prvSwitchTurnRoutine(void);
static void prvSwitchTurnUndo(void);
static void prvSwitchStateTurn(GameState *statevar);
static void prvHandleButtonPress(enum button_num button);
static void prvProcessMessage(MainThread_Message *message);
static void prvRenderState(void);
#endif

#endif