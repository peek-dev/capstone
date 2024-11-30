#ifndef CAPSTONE_LED_H
#define CAPSTONE_LED_H

#include "config.h"

#define NUM_LEDS (82)

// Unlike the APA102c, the SK9822 brightness option changes the current (and
// hence the power draw :D) so it doesn't cause flickering.
typedef struct {
    // Warning: only 5 bits! Top 3 bits ignored.
    uint8_t brightness;
    // Full 8 bits.
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Color;

BaseType_t xLED_clear_board(void);
BaseType_t xLED_set_color(uint8_t num, Color *pColor);
BaseType_t xLED_commit(void);
BaseType_t xLED_save();
BaseType_t xLED_restore();

BaseType_t xLED_Init(void);
void vLED_Thread(void *arg0);

#endif