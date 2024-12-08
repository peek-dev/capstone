#ifndef CAPSTONE_LCD_H
#define CAPSTONE_LCD_H

#include "config.h"
#include "game.h"
#include "clock.h"

void vLCD_RenderTime(uint32_t *times, uint32_t *data);
void vLCD_WriteHardware(uint32_t *data);
void vLCD_Init(void);
void vLCD_RunTestSequence(uint8_t seconds_per_test);
void vLCD_RenderState(uint32_t *data, clock_state state, game_turn turn, uint32_t *times_ms, uint16_t *numbers, uint32_t inc);

// In case I got the bit order wrong.
#define SEVENSEG_A (1 << 0)
#define SEVENSEG_B (1 << 1)
#define SEVENSEG_C (1 << 2)
#define SEVENSEG_D (1 << 3)
#define SEVENSEG_E (1 << 4)
#define SEVENSEG_F (1 << 5)
#define SEVENSEG_G (1 << 6)

#define SEVENSEG_0                                                             \
    (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E |          \
     SEVENSEG_F)
#define SEVENSEG_1 (SEVENSEG_B | SEVENSEG_C)
#define SEVENSEG_2                                                             \
    (SEVENSEG_A | SEVENSEG_B | SEVENSEG_D | SEVENSEG_E | SEVENSEG_G)
#define SEVENSEG_3                                                             \
    (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_G)
#define SEVENSEG_4 (SEVENSEG_B | SEVENSEG_C | SEVENSEG_F | SEVENSEG_G)
#define SEVENSEG_5                                                             \
    (SEVENSEG_A | SEVENSEG_C | SEVENSEG_D | SEVENSEG_F | SEVENSEG_G)
#define SEVENSEG_6                                                             \
    (SEVENSEG_A | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E | SEVENSEG_F |          \
     SEVENSEG_G)
#define SEVENSEG_7 (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C)
#define SEVENSEG_8                                                             \
    (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E |          \
     SEVENSEG_F | SEVENSEG_G)
#define SEVENSEG_9                                                             \
    (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_F |          \
     SEVENSEG_G)

#define CLOCK_A (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_E | SEVENSEG_F | SEVENSEG_G)
#define CLOCK_D (SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E | SEVENSEG_G)
#define CLOCK_E (SEVENSEG_A | SEVENSEG_D | SEVENSEG_E | SEVENSEG_F | SEVENSEG_G)
#define CLOCK_F (SEVENSEG_A | SEVENSEG_E | SEVENSEG_F | SEVENSEG_G)
#define CLOCK_N (SEVENSEG_C | SEVENSEG_E | SEVENSEG_G)
#define CLOCK_O SEVENSEG_0
#define CLOCK_o (SEVENSEG_C | SEVENSEG_D | SEVENSEG_E | SEVENSEG_G)
#define CLOCK_P (SEVENSEG_A | SEVENSEG_B | SEVENSEG_E | SEVENSEG_F | SEVENSEG_G)
#define CLOCK_S SEVENSEG_5
#define CLOCK_U (SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E | SEVENSEG_F)

#define DIGIT_1_1_OFFSET 0
#define DIGIT_1_2_OFFSET 7
#define DIGIT_1_3_OFFSET 14
#define DIGIT_1_4_OFFSET 21
#define DIGIT_1_5_OFFSET (DIGIT_1_1_OFFSET + 32 * 2)
#define DIGIT_1_6_OFFSET (DIGIT_1_2_OFFSET + 32 * 2)
#define DIGIT_2_1_OFFSET (DIGIT_1_1_OFFSET + 32)
#define DIGIT_2_2_OFFSET (DIGIT_1_2_OFFSET + 32)
#define DIGIT_2_3_OFFSET (DIGIT_1_3_OFFSET + 32)
#define DIGIT_2_4_OFFSET (DIGIT_1_4_OFFSET + 32)
#define DIGIT_2_5_OFFSET (DIGIT_1_3_OFFSET + 32 * 2)
#define DIGIT_2_6_OFFSET (DIGIT_1_4_OFFSET + 32 * 2)

#define COL_1_1_OFFSET 28
#define COL_1_2_OFFSET 29
#define COL_2_1_OFFSET (COL_1_1_OFFSET + 32)
#define COL_2_2_OFFSET (COL_1_2_OFFSET + 32)

#endif