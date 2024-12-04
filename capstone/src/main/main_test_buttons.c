#include "config.h"
#include "cmsis_gcc.h"
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
#include "uart_bidir_protocol.h"
#include "button.h"

#define DECLARE_PRIVATE_MAIN_C
#include "main.h"

void mainThread(void *arg0) {
    BaseType_t xReturned;

    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xMain_Init();
    while (xReturned != pdPASS) {}
    vButton_Init();

    xReturned = xPortGetFreeHeapSize();

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

BaseType_t xMain_sensor_update(BoardState *state) { return pdTRUE; }

BaseType_t xMain_button_press_FromISR(enum button_num button,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    MainThread_Message m;
    m.type = main_button_press;
    m.button = button;
    return xQueueSendFromISR(mainQueue, &m, pxHigherPriorityTaskWoken);
}

BaseType_t xMain_uart_message(uint32_t move) {
    MainThread_Message m;
    m.type = main_uart_message;
    m.move = move;
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

__attribute__((noinline)) static void
prvHandleButtonPress(enum button_num button) {
    switch (button) {
    case button_num_black_move:
        xMain_uart_message(0);
        break;
    case button_num_white_move:
        xMain_uart_message(1);
        break;
    case button_num_clock_mode:
        xMain_uart_message(2);
        break;
    case button_num_hint:
        xMain_uart_message(3);
        break;
    case button_num_pause:
        xMain_uart_message(4);
        break;
    case button_num_start_restart:
        xMain_uart_message(5);
        break;
    case button_num_undo:
        xMain_uart_message(6);
        break;
    default:
        xMain_uart_message(7);
        break;
    }
}

static void prvProcessMessage(MainThread_Message *message) {
    volatile NormalMove button;
    switch (message->type) {
    case main_sensor_update:
        break;
    case main_uart_message:
        button = message->move;
        button *= 2;
        break;
    case main_button_press:
        prvHandleButtonPress(message->button);
        break;
    }
}

BaseType_t xMain_sensor_calibration_update(BoardState_Calibration *state) {
    return pdTRUE;
}

BaseType_t xMain_uart_message_FromISR(uint32_t move,
                                      BaseType_t *pxHigherPriorityTaskWoken) {
    return pdTRUE;
}