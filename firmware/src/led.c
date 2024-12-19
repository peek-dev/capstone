/**
 * Copyright (C) <year>  <name of author>
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
#include "config.h"
#include <string.h>
#include <queue.h>
#include "assert.h"

#include "projdefs.h"
#include "ti/driverlib/dl_spi.h"
#include "ti_msp_dl_config.h"
#include "sensor_mutex.h"

#include "led.h"

// 28 is how many squares should be lit for a queen in the middle. So 28 + 2
// (clear and commit) + 1 (leeway/different light for current square) should be
// good.
#define QUEUE_SIZE (28 + 1 + 2)
// Opt: SIMD-style set color by component, send multiple numbers? Possibly half
// as mem-intensive.
static QueueHandle_t ledQueue;
static Color state[NUM_LEDS];
static Color saved_state[NUM_LEDS][2];

enum LED_MsgType {
    led_clear_board,
    led_set_color,
    led_commit,
    led_save,
    led_restore
};

typedef struct {
    enum LED_MsgType type;
    union {
        uint8_t led_num;
        uint8_t save_num;
    };
    Color color;
} LED_Message;

static uint32_t prvPackFrame(Color *pColor) {
    // Initial frame tag, see the SK9822 datasheet.
    uint32_t result = 0b111 << (24 + 5);
    // Datasheet says: blue, green, red.
    result |= pColor->brightness << 24;
    result |= pColor->blue << 16;
    result |= pColor->green << 8;
    result |= pColor->red << 0;

    // Somehow the two chunks are getting flipped in transmission?
    return result;
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
BaseType_t xLED_set_color(uint8_t num, const Color *pColor) {
    LED_Message m;
    m.type = led_set_color;
    m.led_num = num;
    m.color = *pColor;
    return xQueueSend(ledQueue, &m, portMAX_DELAY);
}
static void prvLED_set_color(LED_Message *pMsg) {
    state[pMsg->led_num] = pMsg->color;
}

BaseType_t xLED_commit() {
    LED_Message m;
    m.type = led_commit;
    return xQueueSend(ledQueue, &m, portMAX_DELAY);
}

BaseType_t xLED_save(uint8_t save_num) {
    LED_Message m;
    m.type = led_save;
    m.save_num = save_num;
    return xQueueSend(ledQueue, &m, portMAX_DELAY);
}

BaseType_t xLED_restore(uint8_t save_num) {
    LED_Message m;
    m.type = led_restore;
    m.save_num = save_num;
    return xQueueSend(ledQueue, &m, portMAX_DELAY);
}

static void prvTransmitHalfFrame(uint16_t halfframe) {
    // My own reimplementation of DL_SPI_transmitDataBlocking32,
    // but using yield instead of busy-wait.
    while (DL_SPI_isTXFIFOFull(LED_SPI_INST)) {
        taskYIELD();
    }
    DL_SPI_transmitData16(LED_SPI_INST, halfframe);
}
static void prvTransmitFrame(uint32_t frame) {
    prvTransmitHalfFrame((frame >> 16) & 0xFFFF);
    prvTransmitHalfFrame(frame & 0xFFFF);
}

static void prvLED_commit() {
#if LED_USE_SENSOR_MUTEX
    xSemaphoreTake(sensor_mutex, portMAX_DELAY);
#endif
    // As documented here:
    // https://cpldcpu.wordpress.com/2016/12/13/sk9822-a-clone-of-the-apa102/
    // First, send a zero frame.
    prvTransmitFrame(0);
    // Then, send a frame for every LED.
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        prvTransmitFrame(prvPackFrame(&state[i]));
    }
    // Send another zero frame.
    prvTransmitFrame(0);
    // Send NUM_LEDS/2 bits of zeros. I will do this in frames of 32.
    // (n/2) bits / (32 bits/frame) = n/64 frames, rounded up.
    // To round up, we add 63 and do integer division.
    for (uint8_t i = 0; i < (NUM_LEDS + 63) / 64; i++) {
        prvTransmitFrame(0);
    }
#if LED_USE_SENSOR_MUTEX
    xSemaphoreGive(sensor_mutex);
#endif
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
            case led_save:
                if (message.save_num < 2) {
                    memcpy(&(saved_state[message.save_num]), &state, sizeof(state));
                }
                break;
            case led_restore:
                if (message.save_num < 2) {
                    memcpy(&state, &(saved_state[message.save_num]), sizeof(state));
                }
                break;
            }
        }
    }
}
