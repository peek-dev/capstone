#include <FreeRTOS.h>
#include <queue.h>
#include "portmacro.h"

#include "projdefs.h"
#include "ti/driverlib/dl_spi.h"
#include "ti_msp_dl_config.h"

#include "led.h"

// TODO check that I counted right. 28 is how many squares should be lit for a queen in the middle.
// So 28 + 2 (clear and commit) + 1 (leeway/different light for current square) should be good.
#define QUEUE_SIZE (28+1+2)
#define NUM_LEDS (144)
// Opt: SIMD-style set color by component, send multiple numbers? Possibly half as mem-intensive.
static QueueHandle_t ledQueue;
static Color state[NUM_LEDS];

enum LED_MsgType {
    led_clear_board,
    led_set_color,
    led_commit
};

typedef struct {
// OPT: 8 bits wasted here. Can merge type (2) and led_num (6).
    enum LED_MsgType type;
    uint8_t led_num;
    Color color;
} LED_Message;

__attribute__((noinline)) uint32_t prvPackFrame(Color *pColor) {
    // Initial frame tag, see the SK9822 datasheet.
    uint32_t result = 0b111 << (24+5);
    // Datasheet says: blue, green, red.
    result |= pColor->brightness << 24;
    result |= pColor->blue       << 16;
    result |= pColor->green      << 8;
    result |= pColor->red        << 0;

    // Somehow the two chunks are getting flipped in transmission?
    return (result >> 16) | (result << 16);
}


BaseType_t xLED_clear_board() {
    LED_Message m;
    m.type = led_clear_board;
    return xQueueSend(ledQueue, &m, portMAX_DELAY);
}
static void prvLED_clear_board() {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        state[i].blue = 0;
        state[i].green = 0;
        state[i].red = 0;
        state[i].brightness = 0;
    }
}

// OPT: maybe split this into three to prevent stack passing? Or reference?
BaseType_t xLED_set_color(uint8_t num, Color *pColor) {
    LED_Message m;
    m.type = led_set_color;
    m.led_num = num;
    m.color = *pColor;
    return xQueueSend(ledQueue, &m, portMAX_DELAY);
}
void prvLED_set_color(LED_Message *pMsg) {
    state[pMsg->led_num] = pMsg->color;
}

BaseType_t xLED_commit() {
    LED_Message m;
    m.type = led_commit;
    return xQueueSend(ledQueue, &m, portMAX_DELAY);
}
__attribute__((noinline)) void prvTransmitFrame(uint32_t frame) {
    // My own reimplementation of DL_SPI_transmitDataBlocking32,
    // but using yield instead of busy-wait.
    while (DL_SPI_isTXFIFOFull(LED_SPI_INST)) {
        taskYIELD();
    }
    DL_SPI_transmitData32(LED_SPI_INST, frame);
}
void prvLED_commit() {
    // As documented here: https://cpldcpu.wordpress.com/2016/12/13/sk9822-a-clone-of-the-apa102/
    // First, send a zero frame.
    prvTransmitFrame(0);
    uint32_t temp;
    // Then, send a frame for every LED.
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        prvTransmitFrame(prvPackFrame(&state[i]));
    }
    // Send another zero frame.
    prvTransmitFrame(0);
    // Send NUM_LEDS/2 bits of zeros. I will do this in frames of 32.
    // (n/2) bits / (32 bits/frame) = n/64 frames, rounded up.
    // To round up, we add 63 and do integer division.
    for (uint8_t i = 0; i < (NUM_LEDS+63)/64; i++) {
        prvTransmitFrame(0);
    }
}

BaseType_t xLED_Init(void) {
    prvLED_clear_board();
    prvLED_commit();

    ledQueue = xQueueCreate(QUEUE_SIZE, sizeof(LED_Message));
    if (ledQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

void vLED_Thread(void *arg0) {
    LED_Message message;
    while (1) {
        if (xQueueReceive(ledQueue, &message, portMAX_DELAY) == pdTRUE) {
            switch (message.type) {
                case led_clear_board:
                    prvLED_clear_board();
                    break;
                case led_set_color:
                    prvLED_set_color(&message);
                    break;
                case led_commit:
                    prvLED_commit();
                    break;
            }
        }
    }
}