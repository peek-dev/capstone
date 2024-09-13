#include <stdint.h>
#include "kernel.h"
#include "ti/driverlib/m0p/dl_systick.h" // Include TI driver header for SysTick

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

tcb SystemThreads[THREADS_MAX];
tcb* CurrentThread;

/*
 * Add "static" qualifier to make this global within kernel.c but 
 * otherwise inaccessible to other files.
 */
static uint8_t ActiveThreads;

int32_t ThreadStacks[THREADS_MAX][STACK_SIZE];

/**
 * Disable system interrupts.
 */
void DisableInterrupts(void) {
    __asm(
            "\
            cpsid i; \
            bx lr; \
            "
         );
}

/**
 * Enable system interrupts.
 */
void EnableInterrupts(void) {
    __asm(
            "\
            cpsie i; \
            bx lr; \
            "
         );
}

/**
 * Implement the relevant system policy (by default, round robin allowing for 
 * sleeping threads) to decide which thread runs next.
 */
void Scheduler(void) {
    while (CurrentThread->status != THREAD_ALIVE) {
        CurrentThread = CurrentThread->next;
    }
}

// Directly lifted from the kernel in ECE 4501: Advanced Embedded Computing 
// Systems at the University of Virginia.
// Presumably used from Valvano's textbook; copyright notices given here and 
// in kernel.h as such.
void SetInitialStack(int ThreadIndex){
  SystemThreads[ThreadIndex].sp = &ThreadStacks[ThreadIndex][STACK_SIZE-16]; // thread stack pointer
  ThreadStacks[ThreadIndex][STACK_SIZE-1] = 0x01000000;   // thumb bit
  ThreadStacks[ThreadIndex][STACK_SIZE-3] = 0x14141414;   // R14
  ThreadStacks[ThreadIndex][STACK_SIZE-4] = 0x12121212;   // R12
  ThreadStacks[ThreadIndex][STACK_SIZE-5] = 0x03030303;   // R3
  ThreadStacks[ThreadIndex][STACK_SIZE-6] = 0x02020202;   // R2
  ThreadStacks[ThreadIndex][STACK_SIZE-7] = 0x01010101;   // R1
  ThreadStacks[ThreadIndex][STACK_SIZE-8] = 0x00000000;   // R0
  ThreadStacks[ThreadIndex][STACK_SIZE-9] = 0x11111111;   // R11
  ThreadStacks[ThreadIndex][STACK_SIZE-10] = 0x10101010;  // R10
  ThreadStacks[ThreadIndex][STACK_SIZE-11] = 0x09090909;  // R9
  ThreadStacks[ThreadIndex][STACK_SIZE-12] = 0x08080808;  // R8
  ThreadStacks[ThreadIndex][STACK_SIZE-13] = 0x07070707;  // R7
  ThreadStacks[ThreadIndex][STACK_SIZE-14] = 0x06060606;  // R6
  ThreadStacks[ThreadIndex][STACK_SIZE-15] = 0x05050505;  // R5
  ThreadStacks[ThreadIndex][STACK_SIZE-16] = 0x04040404;  // R4
}

/**
 * Add a thread to the system's list of thread control blocks (TCBs), giving 
 * the thread a function pointer for its relevant "main" to execute.
 *
 * Returns: 0 if successful or -1 if unsuccessful.
 */
int8_t AddThread(void (*task)(void)) {
    DisableInterrupts();

    if (ActiveThreads == THREADS_MAX) {
        DisableInterrupts();
        return -1;
    }

    EnableInterrupts();

    for (int index = 0; index < THREADS_MAX; index += 1) {
        if ((SystemThreads[index].status == THREAD_BLANK) || (SystemThreads[index].status == THREAD_EXITED)) {
            tcb* NewThread = &(SystemThreads[index]);
            SetInitialStack(index); 
            ThreadStacks[index][STACK_SIZE - 2] = (int32_t)(task); // Copy new thread task PC (function pointer value) to new thread stack
                                                    
            NewThread.status = THREAD_ALIVE;
            NewThread->prev = CurrentThread;

            // Ensure CurrentThread (pointer to currently running thread's TCB)
            // is defined and not NULL. If already not NULL, no problem; if 
            // NULL, the new thread is the only one that exists on the system,
            // so define CurrentThread as such.
            if (CurrentThread != NULL) {
                CurrentThread->next = NewThread;
                NewThread->next = CurrentThread->next;
            } else {
                CurrentThread = NewThread;
                NewThread->next = CurrentThread;
            }

            ActiveThreads += 1;

            EnableInterrupts();
            return 1;
        }
    }

    EnableInterrupts();

    return -1;
}

/**
 * Manually trigger a context switch and run the scheduler to potentially allow
 * another thread to run.
 *
 * NOTE: this function does not return in the calling thread.
 */
void ThreadYield(void) {
    // Invoke the MSPM0-specific TI driver to clear SysTick (and trigger a 
    // SysTick interrupt)
    DL_SYSTICK_resetValue();
}

/**
 * Mark the calling thread as "dead" (i.e., exited), then trigger a context 
 * switch to allow the kernel to reclaim bookkeeping resources for this thread
 * and possibly run another thread.
 *
 * NOTE: this function does not return.
 */
void ThreadExit(void) {
    DisableInterrupts();
    CurrentThread->prev->next = CurrentThread->next;
    CurrentThread->next->prev = CurrentThread->prev;
    CurrentThread->status = THREAD_EXITED;
    EnableInterrupts();
    ThreadYield(); // yield the CPU and provoke a context switch
}

/**
 * Mark the calling thread as asleep for `ms` milliseconds, then invoke a context switch to allow another thread to potentially run.
 *
 * NOTE: this function does not return in the caller.
 */
void ThreadSleep(uint32_t ms) {
    DisableInterrupts();
    CurrentThread->sleep_ct = ms;
    CurrentThread->status = THREAD_SLEEPING;
    EnableInterrupts();
    ThreadYield(); // yield the CPU and provoke a context switch
}

/**
 * A "private" function to initialize OS data structures and related state.
 */
void OS_Init(void) {
    for (int index = 0; index < THREADS_MAX; index += 1) {
        SystemThreads[index].sp = NULL;
        SystemThreads[index].prev = NULL;
        SystemThreads[index].next = NULL;
        SystemThreads[index].status = THREAD_BLANK;
    }

    ActiveThreads = 0;
}

/**
 * A wrapper function to set up OS state, utilities, and timing, then begin 
 * executing a thread.
 *
 * NOTE: this function never returns.
 */
void OS_Launch(uint32_t TimeSlice) {
    // TODO: program SysTick with TimeSlice
    // TODO: jump to first thread
}

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
void Panic(void) {
    DisableInterrupts();
    // TODO: potentially set LEDs to all-on or some other visual indicator
    while (1);
}

