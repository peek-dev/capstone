#include "config.h"
#include <task.h>
#include <string.h>
#include <queue.h>

#include "main.h"
#include "game.h"
#include "portmacro.h"
#include "projdefs.h"
#include "sensor.h"

enum MainThread_MsgType { main_sensor_update, main_update_mode };

typedef struct {
    enum MainThread_MsgType type;
    union {
        BoardState state;
        Main_ThreadMode mode;
    };
    union {
        uint16_t num_requests;
        uint16_t bin_num;
    };
} MainThread_Message;

extern BoardState correct;

static GameState state;

// IDK, maybe change this later. Profiling?
#define QUEUE_SIZE 10
static QueueHandle_t mainQueue;

static TaskHandle_t thread_sensor;
static uint32_t error_count[N_STDDEV_BINS][main_thread_finished] = {0};
static uint32_t num_requests[N_STDDEV_BINS] = {0};
static Main_ThreadMode mode = 0;
static uint16_t stddev_index = 0;

BaseType_t xMain_Init(void);
static void prvProcessMessage(MainThread_Message *message);

void mainThread(void *arg0) {
    MainThread_Message message;
    BaseType_t xReturned;

    /* Call driver init functions */
    xReturned = xMain_Init();
    while (xReturned != pdPASS) {}
    xReturned = xSensor_Init();
    while (xReturned != pdPASS) {}
    xReturned =
        xTaskCreate(vSensor_Thread, "Sensor", 3 * configMINIMAL_STACK_SIZE,
                    NULL, 2, &thread_sensor);
    while (xReturned != pdPASS) {}

    MAKEVISIBLE BaseType_t mem = xPortGetFreeHeapSize();
    while (1) {
        if (mode == main_thread_finished) {
            while (1) {}
        }
        if (xQueueReceive(mainQueue, &message, portMAX_DELAY) == pdTRUE) {
            prvProcessMessage(&message);
        }
    }

    vTaskDelete(NULL);
}

BaseType_t xMain_Init(void) {
    vBoardSetDefault(&state.true_state);
    mainQueue = xQueueCreate(QUEUE_SIZE, sizeof(MainThread_Message));
    if (mainQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

BaseType_t xMain_sensor_update(BoardState *state, uint16_t nreq) {
    MainThread_Message m;
    m.type = main_sensor_update;
    memcpy(&m.state, state, sizeof(BoardState));
    m.num_requests = nreq;
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

BaseType_t xMain_change_mode(Main_ThreadMode mode, uint16_t stddev_i) {
    MainThread_Message m;
    m.type = main_update_mode;
    m.mode = mode;
    m.bin_num = stddev_i;
    return xQueueSend(mainQueue, &m, portMAX_DELAY);
}

static void prvCheckState() {
    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t col = 0; col < 8; col++) {
            if (xGetSquare(&state.last_measured_state, row, col) !=
                xGetSquare(&state.true_state, row, col)) {
                error_count[stddev_index][mode]++;
            }
        }
    }
}

static void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case main_sensor_update:
        memcpy(&state.last_measured_state, &(message->state),
               sizeof(BoardState));
        prvCheckState();
        if (mode == main_thread_requests) {
            num_requests[stddev_index] += message->num_requests;
        }
        break;
    case main_update_mode:
        mode = message->mode;
        stddev_index = message->bin_num;
    default:
        break;
    }
}