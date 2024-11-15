#ifndef CAPSTONE_MAIN_H
#define CAPSTONE_MAIN_H

#include "game.h"
#include "button.h"

BaseType_t xMain_sensor_update(BoardState *state);
BaseType_t xMain_button_press(enum button_num button);
BaseType_t xMain_uart_message(uint32_t move);

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
        // TODO: this is sooo inefficient. 32 bytes vs 4?
        BoardState state;
        // TODO: buttons, uart (separate undo and new move)
        enum button_num button;
        uint32_t move;
    };
} MainThread_Message;

#define MAX_POSSIBLE_MOVES 256
static NormalMove prvPossibleMoves[MAX_POSSIBLE_MOVES];
static uint8_t prvPossibleMovesLen = 0;
static uint8_t prvCurrentMoveIndex = 0;
static GameState state;
// IDK, maybe change this later. Profiling?
#define QUEUE_SIZE 10
static QueueHandle_t mainQueue;

BaseType_t xMain_Init(void);
static void prvSwitchTurnRoutine(void);
static void prvSwitchStateTurn(GameState *statevar);
void prvHandleButtonPress(enum button_num button);
void prvProcessMessage(MainThread_Message *message);
#endif

#endif