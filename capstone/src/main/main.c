#include "config.h"
#include <task.h>
#include <string.h>
#include <queue.h>


#include "main.h"
#include "sensor.h"

enum MainThread_MsgType {
    main_sensor_update
};

typedef struct {
    enum MainThread_MsgType type;
    union {
        BoardState state;
    };
} MainThread_Message;

static GameState state;

// IDK, maybe change this later. Profiling?
#define QUEUE_SIZE 10
static QueueHandle_t mainQueue;

static TaskHandle_t thread_sensor;

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
    xReturned = xTaskCreate(vSensor_Thread, "Sensor", configMINIMAL_STACK_SIZE,
                            NULL, 2, &thread_sensor);
    while (xReturned != pdPASS) {}

    MAKEVISIBLE BaseType_t mem = xPortGetFreeHeapSize();
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

static void prvProcessMessage(MainThread_Message *message) {
    switch (message->type) {
    case main_sensor_update:
        memcpy(&state.last_measured_state, &(message->state),
               sizeof(BoardState));
        break;
    default:
        break;
    }
}