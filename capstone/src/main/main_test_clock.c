#include <FreeRTOS.h>
#include <task.h>

#include "chess.h"
#include "config.h"
#include "game.h"
#include "clock.h"
#include "portmacro.h"

#define MAX_POSSIBLE_MOVES 256
static NormalMove prvPossibleMoves[MAX_POSSIBLE_MOVES];
static uint8_t prvLastMoveIndex = 0;
static BoardState prvLastGoodState;

enum MainThread_MsgType {
    chess_sensor_update,
    chess_button_press,
    chess_uart_packet
};

typedef struct {
    enum MainThread_MsgType type;
    union {
        // TODO: this is sooo inefficient. 32 bytes vs 4?
        struct {
            BoardState state;
        };
        // TODO: buttons, uart (separate undo and new move)
    } data;
} MainThread_Message;

void mainThread(void *arg0) {
    TaskHandle_t thread_clock;
    BaseType_t xReturned;

    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {
    }

    xReturned = xPortGetFreeHeapSize();

    // Make a thread to recieve messages.
    xReturned = xTaskCreate(vClock_Thread, "Clock", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_clock);
    // Loop and get stuck here if we fail.
    while (xReturned != pdPASS) {
    }
    xReturned = xPortGetFreeHeapSize();

    xReturned = xClock_run_test(1);
    while (xReturned != pdPASS) {
    }
    vTaskDelete(NULL);
}