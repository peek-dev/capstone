#include <stdint.h>
#include "kernel.h"

#define THREADS_MAX 16
tcb SystemThreads[THREADS_MAX] = {0};
uint8_t ActiveThreads;

tcb* CurrentThread;

void DisableInterrupts(void) {
    __asm(
            "cpsid i; \
            bx lr;"
         );
}

void EnableInterrupts(void) {
    __asm(
            "cpsie i; \
            bx lr;"
         );
}

void Scheduler(void) {
    while (CurrentThread->status != THREAD_ALIVE) {
        CurrentThread = CurrentThread->next;
    }
}

int8_t AddThread();

void ThreadYield(void) {
    // TODO: write to SysTick to trigger a SysTick interrupt
}

void ThreadExit(void) {
    DisableInterrupts();
    CurrentThread->prev->next = CurrentThread->next;
    CurrentThread->next->prev = CurrentThread->prev;
    CurrentThread->status = THREAD_EXITED;
    EnableInterrupts();
    // TODO: write to SysTick to trigger a SysTick interrupt
}

void ThreadSleep(uint32_t ms) {
    DisableInterrupts();
    CurrentThread->sleep_ct = ms;
    CurrentThread->status = THREAD_SLEEPING;
    EnableInterrupts();
    // TODO: write to SysTick to trigger a SysTick interrupt
}

void Panic(void) {
    DisableInterrupts();
    // TODO: potentially set LEDs to all-on or some other visual indicator
    while (1);
}
