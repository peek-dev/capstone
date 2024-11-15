#ifndef __CAPSTONE_UART_THREAD_H__
#define __CAPSTONE_UART_THREAD_H__

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h> // For FreeRTOS semaphores

typedef struct {
    // TODO: make fields pointers if not already declared as pointers (???)
    QueueHandle_t main_to_uart;
    SemaphoreHandle_t mtu_lock; // Semaphores intended to be **binary** semaphores---what FreeRTOS docs suggest as superior choice for inter-task synchronization
    QueueHandle_t uart_to_main;
    SemaphoreHandle_t utm_lock;
} UART_arg;

#endif
