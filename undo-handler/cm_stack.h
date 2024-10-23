#ifndef __CM_STACK_H__
#define __CM_STACK_H__

#include <stdint.h>

/**
 * A friendly, clearer alias.
 */
typedef uint32_t chessmove_t;

typedef struct cm_node_t_struct cm_node_t;

/**
 * A singly-linked list node containing chessmove data (in 32-bit UART words 
 * as negotiated in this project's bespoke UART protocol between the RPi5 and
 * the MSPM0) and a pointer to the next node for linking.
 */
typedef struct cm_node_t_struct {
    chessmove_t move;
    cm_node_t* next;
} cm_node_t;

typedef struct cm_stack_t_struct cm_stack_t;

/**
 * A simple singly-linked list implementation of a stack for chessmove data, 
 * containing fields for the top node (for easy constant-time pushing/popping)
 * and nodecount (for corner case handling).
 */
typedef struct cm_stack_t_struct {
    cm_node_t* top;
    int nodecount;
} cm_stack_t;

/**
 * Initialize a chessmove stack on the heap and return a pointer to a valid 
 * stack.
 */
cm_stack_t* cm_stack_init(void);

/**
 * Push a 32-bit word to a stack, invoking necessary internal logic to wrap the
 * data in a node and adjust stack state.
 */
void cm_stack_push(cm_stack_t* stack, chessmove_t new_move);

/**
 * Pop the top node from a stack and return the data from that node as a 
 * uint32_t (the type that "chessmove_t" aliases).
 */
uint32_t cm_stack_pop(cm_stack_t* stack);

/**
 * Clean up and free a stack and its associated heap space.
 */
void cm_stack_destroy(cm_stack_t* stack);

#endif
