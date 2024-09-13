#include <stdint.h>

#ifndef __KERNEL_H__
#define __KERNEL_H__

#ifndef NULL
#define NULL ((void*) 0)
#endif

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

#define THREADS_MAX     16
#define STACK_SIZE      64      // maximum number of 32-bit words in a thread's stack. Effectively marks a 2 KiB maximum stack per thread.

#define THREAD_ALIVE    ((int8_t) 1)
#define THREAD_SLEEPING ((int8_t) -1)
#define THREAD_EXITED   ((int8_t) 0)
#define THREAD_BLANK    THREAD_EXITED

// By default, set macros based on the use of the maximum-speed 80 MHz clock
// on the MSPM0G3507.
#ifndef USE_80MHZ_CLK
    #ifndef USE_32MHZ_CLK
        #define USE_80MHZ_CLK 1
    #endif
#endif

// Define time slice macros for easy reference based on selected clock speed.
#ifdef USE_80MHZ_CLK
#define TIMESLICE_1MS   ((uint32_t) 80000)
#elif defined (USE_32MHZ_CLK)
#define TIMESLICE_1MS   ((uint32_t) 32000)
#endif

#define TIMESLICE_2MS   (2 * TIMESLICE_1MS)
#define TIMESLICE_5MS   (5 * TIMESLICE_1MS)
#define TIMESLICE_10MS  (10 * TIMESLICE_1MS)

/*
 * The thread control block struct, defining a thread's stack pointer, 
 * previous and next threads in the entire list of threads, and a sleep count
 * to be internally maintained to track sleeping threads.
 */
typedef struct tcb_struct tcb;

typedef struct tcb_struct {
    int32_t* sp;
    tcb* prev;
    tcb* next; 
    uint32_t sleep_ct;
    /**
     * One of three thread statuses possible: 
     * - alive (available to be scheduled); 
     * - sleeping (available to be scheduled at some point in the future but currently unavailable); 
     * - dead (exited and unavailable to ever be scheduled again)
     */
    int8_t status; 
} tcb;

/** 
 * Disable system interrupts.
 *
 * NOTE: this function is a smaller wrapper from assembly routines that was
 * originally presented in Jonathan Valvano's textbook on RTOS development for 
 * ARM Cortex-M microcontrollers. See the copyright notice at the top of this 
 * file for more information.
 */
void DisableInterrupts(void);

/**
 * Enable system interrupts.
 *
 * NOTE: this function is a smaller wrapper from assembly routines that was
 * originally presented in Jonathan Valvano's textbook on RTOS development for 
 * ARM Cortex-M microcontrollers. See the copyright notice at the top of this 
 * file for more information.
 */
void EnableInterrupts(void);

/**
 * Schedule the next thread to run on the system.
 *
 * NOTE: this function should **never** be called directly; the SysTick Handler
 * or another hardware timer interrupt should instead be left to call this 
 * function as part of the handler execution.
 */
void Scheduler(void);

/**
 * Add a thread to the system's list of thread control blocks (TCBs), giving 
 * the thread a function pointer for its relevant "main" to execute.
 *
 * Returns: 0 if successful or -1 if unsuccessful.
 */
int8_t AddThread(void (*task)(void));

/**
 * Manually trigger a context switch and run the scheduler to potentially allow
 * another thread to run.
 *
 * NOTE: this function does not return in the calling thread.
 */
void ThreadYield(void);

/**
 * Mark the calling thread as "dead" (i.e., exited), then trigger a context 
 * switch to allow the kernel to reclaim bookkeeping resources for this thread
 * and possibly run another thread.
 *
 * NOTE: this function does not return.
 */
void ThreadExit(void);

/**
 * Mark the calling thread as asleep for `ms` milliseconds, then invoke a context switch to allow another thread to potentially run.
 *
 * NOTE: this function does not return in the caller.
 */
void ThreadSleep(uint32_t ms);

/**
 * A wrapper function to set up OS state, utilities, and timing, then begin 
 * executing a thread.
 *
 * NOTE: this function never returns.
 */
void OS_Launch(uint32_t TimeSlice);

/**
 * A "sentinel" routine to unambiguously signal an unrecoverable machine state/
 * fault, etc.
 *
 * Primarily for debugging purposes. Functions can call this to indicate a 
 * fault condition in more broadly definitions than the onboard fault handlers
 * allow.
 *
 * NOTE: this function will never return and will require a power 
 * reset/brownout reset/etc. on the board to be exited from.
 */
void Panic(void);

#endif /* __KERNEL_H__ */
