/**
 * Copyright (C) 2024 John E. Berberian, Jr.
 *
 * flash_square.c: timing thread for flashing specific squares with different periods.
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
#include <queue.h>
#include <task.h>
#include "led.h"
#include "projdefs.h"
#include "flash_square.h"

#define FLASH_QUEUE_SIZE     (64)
#define MAX_FLASHING_SQUARES (64)

static uint8_t flash_led_nums[MAX_FLASHING_SQUARES];
// A value of 0 indicates disabled.
static uint16_t halfperiod[MAX_FLASHING_SQUARES] = {0};
static uint16_t remaining[MAX_FLASHING_SQUARES];
static BaseType_t currently_on[MAX_FLASHING_SQUARES];
static Color colors[MAX_FLASHING_SQUARES];
static uint8_t n_enabled = 0;
static const Color off = {
    .brightness = 0,
    .red = 0,
    .green = 0,
    .blue = 0,
};

typedef struct {
    Color color;
    uint16_t half_period;
    uint8_t led_num;
    uint8_t disable_all;
} FlashSquare_Message;

static QueueHandle_t prvFlashQueue = NULL;

BaseType_t xFlashSquare_Init() {
    prvFlashQueue = xQueueCreate(FLASH_QUEUE_SIZE, sizeof(FlashSquare_Message));
    if (prvFlashQueue == NULL) {
        return pdFALSE;
    }
    return pdTRUE;
}

BaseType_t xFlashSquare_Enable(uint8_t led_num, uint16_t half_period_ticks,
                               Color color) {
    FlashSquare_Message m = {
        .led_num = led_num,
        .half_period = half_period_ticks,
        .color = color,
        .disable_all = 0,
    };
    return xQueueSend(prvFlashQueue, &m, portMAX_DELAY);
}

BaseType_t xFlashSquare_Disable(uint8_t led_num) {
    FlashSquare_Message m = {
        .led_num = led_num,
        .half_period = 0,
        .disable_all = 0,
    };
    return xQueueSend(prvFlashQueue, &m, portMAX_DELAY);
}

BaseType_t xFlashSquare_DisableAll() {
    FlashSquare_Message m = {
        .disable_all = 1,
    };
    return xQueueSend(prvFlashQueue, &m, portMAX_DELAY);
}

static uint8_t prvMinDelay() {
    uint16_t min = 0xFFFF;
    uint8_t index = 0;
    for (uint8_t i = 0; i < MAX_FLASHING_SQUARES; i++) {
        if (halfperiod[i] != 0 && remaining[i] < min) {
            min = remaining[i];
            index = i;
        }
    }
    return index;
}

static void prvProcessTimes(TickType_t ticks) {
    BaseType_t any_changed = pdFALSE;
    for (int i = 0; i < MAX_FLASHING_SQUARES; i++) {
        if (halfperiod[i] != 0) {
            remaining[i] -= ticks;
            if (remaining[i] == 0) {
                remaining[i] += halfperiod[i];
                const Color *c;
                if (currently_on[i] == pdTRUE) {
                    c = &off;
                    currently_on[i] = pdFALSE;
                } else {
                    c = &colors[i];
                    currently_on[i] = pdTRUE;
                }
                xLED_set_color(flash_led_nums[i], c);
                any_changed = pdTRUE;
            }
        }
    }
    if (any_changed == pdTRUE) {
        xLED_commit();
    }
}

void vFlashSquare_Thread(void *arg0) {
    FlashSquare_Message m;
    TickType_t next_delay;
    TickType_t t1, t2;

    while (1) {
        if (n_enabled != 0) {
            next_delay = remaining[prvMinDelay()];
        } else {
            next_delay = portMAX_DELAY;
        }
        t1 = xTaskGetTickCount();
        BaseType_t recieved = xQueueReceive(prvFlashQueue, &m, next_delay);
        if (recieved == pdFALSE) {
            // Timed out. Deduct timeout.
            prvProcessTimes(next_delay);
        } else if (n_enabled != 0) {
            // Measure time. This will break if you run our system
            // for 50 days, for one second when the time rolls over. Be advised.
            t2 = xTaskGetTickCount();
            prvProcessTimes(t2 - t1);
        }
        if (recieved == pdTRUE) {
            if (m.disable_all != 0) {
                uint8_t i = 0;
                for (i = 0; i < MAX_FLASHING_SQUARES && n_enabled > 0; i++) {
                    if (halfperiod[i] != 0) {
                        xLED_set_color(flash_led_nums[i], &off);
                        n_enabled--;
                    }
                    halfperiod[i] = 0;
                }
                if (i != 0) {
                    xLED_commit();
                }
                n_enabled = 0;
            } else {
                if (m.half_period == 0) {
                    // disabling a square.
                    xLED_set_color(m.led_num, &off);
                } else {
                    // enabling a square?
                    //xLED_set_color(m.led_num, &m.color);
                }
                for (uint8_t i = 0; i < MAX_FLASHING_SQUARES; i++) {
                    if ((m.half_period != 0 && halfperiod[i] == 0) ||
                        (m.half_period == 0 && halfperiod[i] != 0 &&
                         flash_led_nums[i] == m.led_num)) {
                        halfperiod[i] = m.half_period;
                        // The rest of the values are ignored if the period is
                        // 0.
                        remaining[i] = m.half_period;
                        currently_on[i] = pdFALSE;
                        colors[i] = m.color;
                        flash_led_nums[i] = m.led_num;
                        n_enabled += 1 - 2 * (m.half_period == 0);
                        break;
                    }
                }
                xLED_commit();
            }
        }
    }
}
