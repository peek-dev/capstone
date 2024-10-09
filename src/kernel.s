THUMB
AREA |.text|, CODE, READONLY, ALIGN=2

EXTERN CurrentThread
IMPORT Scheduler
EXPORT StartOS
EXPORT SysTick_Handler

SysTick_Handler
    CPSID I               ; Disable interrupts at the processor level
    PUSH {R2-R7}         ; Save current thread registers to stack
    LDR R0, =CurrentThread      ;
    LDR R1, [R0]
    STR SP, [R1]        ;
    PUSH {R0,LR}
    BL Scheduler       ; 
    POP {R0,LR}        ;
    LDR R1, [R0]
    LDR SP, [R1]
    POP {R2-R7}         ;
    CPSIE I               ; Enable interrupts at the processor level

