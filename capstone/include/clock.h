#ifndef CAPSTONE_CLOCK_H
#define CAPSTONE_CLOCK_H

#include "config.h"

BaseType_t xClock_Init(void);
void vClock_Thread(void *arg0);

BaseType_t xClock_run_test(uint8_t seconds_per_test);
BaseType_t xClock_switch_turn();
BaseType_t xClock_stop_clock();
BaseType_t xClock_start_clock();
BaseType_t xClock_set_times(uint32_t *times_ms);

// In case I got the bit order wrong.
#define SEVENSEG_A (1 << 0)
#define SEVENSEG_B (1 << 1)
#define SEVENSEG_C (1 << 2)
#define SEVENSEG_D (1 << 3)
#define SEVENSEG_E (1 << 4)
#define SEVENSEG_F (1 << 5)
#define SEVENSEG_G (1 << 6)

#define CLOCK_0                                                                \
    (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E |          \
     SEVENSEG_F)
#define CLOCK_1 (SEVENSEG_B | SEVENSEG_C)
#define CLOCK_2 (SEVENSEG_A | SEVENSEG_B | SEVENSEG_D | SEVENSEG_E | SEVENSEG_G)
#define CLOCK_3 (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_G)
#define CLOCK_4 (SEVENSEG_B | SEVENSEG_C | SEVENSEG_F | SEVENSEG_G)
#define CLOCK_5 (SEVENSEG_A | SEVENSEG_C | SEVENSEG_D | SEVENSEG_F | SEVENSEG_G)
#define CLOCK_6                                                                \
    (SEVENSEG_A | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E | SEVENSEG_F |          \
     SEVENSEG_G)
#define CLOCK_7 (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C)
#define CLOCK_8                                                                \
    (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E |          \
     SEVENSEG_F | SEVENSEG_G)
#define CLOCK_9                                                                \
    (SEVENSEG_A | SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_F |          \
     SEVENSEG_G)

#define CLOCK_U (SEVENSEG_C | SEVENSEG_D | SEVENSEG_E)
#define CLOCK_N (SEVENSEG_C | SEVENSEG_E | SEVENSEG_G)
#define CLOCK_D (SEVENSEG_B | SEVENSEG_C | SEVENSEG_D | SEVENSEG_E | SEVENSEG_G)
#define CLOCK_O (SEVENSEG_C | SEVENSEG_D | SEVENSEG_E | SEVENSEG_G)

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
