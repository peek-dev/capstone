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
#include "uart_bidir_protocol.h"
#include "led_translation.h"
#include "button.h"
#include "sensor_mutex.h"

#define DECLARE_PRIVATE_MAIN_C
#include "main.h"

void mainThread(void *arg0) {
    /* FIXME: declare task handle for UART task */
    TaskHandle_t thread_led, thread_sensor;
    BaseType_t xReturned;

    xReturned = xMain_Init();
    while (xReturned != pdPASS) {}
    
    xReturned = xPortGetFreeHeapSize();

    /* Call driver init functions */
    xReturned = xClock_Init();
    while (xReturned != pdPASS) {}
    xReturned = xLED_Init();
    while (xReturned != pdPASS) {}
    xReturned = xSensor_Init();
    while (xReturned != pdPASS) {}
    vButton_Init();

    xReturned = xPortGetFreeHeapSize();

    xReturned = xTaskCreate(vLED_Thread, "LED", configMINIMAL_STACK_SIZE, NULL,
                            2, &thread_led);
    while (xReturned != pdPASS) {}
    xReturned = xTaskCreate(vSensor_Thread, "Sensor", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_sensor);
    while (xReturned != pdPASS) {}


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
    sensor_mutex = xSemaphoreCreateMutex();
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

static void prvHandleButtonPress(enum button_num button) {
    // If it's the turn-switch button:
    switch (button) {
    case button_num_white_move:
        prvMovesLen = (prvMovesLen + 63) % 64;
        break;
    case button_num_black_move:
        prvMovesLen = (prvMovesLen + 1) % 64;
        break;
    default:
        break;
    }
}

static void prvRenderState(void) {
    uint8_t row = prvMovesLen / 8;
    uint8_t col = prvMovesLen % 8;
    Color color = {.brightness = 31, .red = 255, .green = 255, .blue = 255};
    // Illuminate the square and the piece type on that square.
    xLED_clear_board();
    xLED_set_color(LEDTrans_Square(row, col), &color);
    PieceType piece = xGetSquare(&state.last_measured_state, row, col);
    ZeroToTwoInts pieceOutlines = LEDTrans_Ptype(piece);
    for (uint8_t i = 0; i < pieceOutlines.len; i++) {
        xLED_set_color(pieceOutlines.data[i], &color);
    }
    xLED_commit();
}

static void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case main_sensor_update:
        memcpy(&state.last_measured_state, &(message->state),
               sizeof(BoardState));
        prvRenderState();
        break;
    case main_uart_message:
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