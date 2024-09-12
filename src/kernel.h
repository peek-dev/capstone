#include <stdint.h>

#ifndef __KERNEL_H__
#define __KERNEL_H__

// Code here is inspired by/reused from Jonathan Valvano's "Embedded Systems:
// Real Time Interfacing to ARM Cortex M Microcontrollers".

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Programs 4.4 through 4.12, section 4.2
 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

/*
 * The thread control block struct, defining a thread's stack pointer, 
 * previous and next threads in the entire list of threads, and a sleep count
 * to be internally maintained to track sleeping threads.
 */
typedef struct tcb_struct tcb;

typedef struct tcb_struct {
    uint32_t* thread_sp;
    tcb* prev;
    tcb* next; 
    uint32_t sleep_ct;
} tcb;

/**
 * Save the current mask of pending interrupts and return as a uint32_t.
 * Then, disable system interrupts.
 *
 * NOTE: this function is a smaller wrapper from assembly routines that was originally 
 */
uint32_t DisableInterrupts(void);

/**
 * Restore the mask of pending interrupts returned from a previous 
 * DisableInterrupts() call, then enable system interrupts.
 */
void EnableInterrupts(uint32_t InterruptMask);

#endif /* __KERNEL_H__ */
