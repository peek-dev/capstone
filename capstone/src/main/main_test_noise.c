#include "config.h"
#include <task.h>
#include "portmacro.h"
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

static BoardState starting_state;

void mainThread(void *arg0) {
    TaskHandle_t thread_clock, thread_led, thread_sensor;
    BaseType_t xReturned;

    /* Call driver init functions */
    xReturned = xSensor_Init();
    while (xReturned != pdPASS) {
    }
    xReturned = xMain_Init();
    while (xReturned != pdPASS) {
    }
    
    xReturned = xTaskCreate(vSensor_Thread, "Sensor", configMINIMAL_STACK_SIZE,
                            NULL, 5, &thread_sensor);
    while (xReturned != pdPASS) {
    }

    MainThread_Message message;
    while (1) {
        if (xQueueReceive(mainQueue, &message, portMAX_DELAY) == pdTRUE) {
            prvProcessMessage(&message);
        }
    }

    vTaskDelete(NULL);
}

static void prvSetInitialBoardState(BoardState *brd) {
    vSetSquare(brd, 0, 0, WhiteRook);
    vSetSquare(brd, 0, 7, WhiteRook);
    vSetSquare(brd, 0, 1, WhiteKnight);
    vSetSquare(brd, 0, 6, WhiteKnight);
    vSetSquare(brd, 0, 2, WhiteBishop);
    vSetSquare(brd, 0, 5, WhiteBishop);
    vSetSquare(brd, 0, 3, WhiteQueen);
    vSetSquare(brd, 0, 4, WhiteKing);

    vSetSquare(brd, 7, 0, BlackRook);
    vSetSquare(brd, 7, 7, BlackRook);
    vSetSquare(brd, 7, 1, BlackKnight);
    vSetSquare(brd, 7, 6, BlackKnight);
    vSetSquare(brd, 7, 2, BlackBishop);
    vSetSquare(brd, 7, 5, BlackBishop);
    vSetSquare(brd, 7, 3, BlackQueen);
    vSetSquare(brd, 7, 4, BlackKing);

    for (uint8_t i = 0; i < 8; i++) {
        vSetSquare(brd, 1, i, WhitePawn);
        vSetSquare(brd, 6, i, BlackPawn);
        for (uint8_t j = 2; j < 6; j++) vSetSquare(brd, j, i, WhitePawn);
    }
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
    return pdTRUE;
}

BaseType_t xMain_uart_message(uint32_t move) {
    return pdTRUE;
}

void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case main_sensor_update:
        memcpy(&state.last_measured_state, &(message->state),
               sizeof(BoardState));
        break;
    case main_uart_message:
        break;
    case main_button_press:
        break;
    }
}