#include <stdlib.h>
#include "cm_stack.h"

cm_stack_t* cm_stack_init(void) {
    cm_stack_t* new_stack = (cm_stack_t*) malloc(1 * sizeof(cm_stack_t));

    if (new_stack == NULL) {
        return NULL;
    }

    new_stack->top = NULL;
    new_stack->nodecount = 0;

    return new_stack;
}

static cm_node_t* cm_node_init(chessmove_t move) {
    cm_node_t* new_node = (cm_node_t*) malloc(1 * sizeof(cm_node_t));

    if (new_node == NULL) {
        return NULL;
    }

    new_node->move = move;
    new_node->next = NULL;

    return new_node;
}

void cm_stack_push(cm_stack_t* stack, chessmove_t new_move) {
    cm_node_t* new_node = cm_node_init(new_move);

    if (new_node == NULL) {
        return;
    }

    new_node->next = stack->top;
    stack->top = new_node;
    stack->nodecount += 1;
}

uint32_t cm_stack_pop(cm_stack_t* stack) {
    if (stack->top == NULL) {
        return 0xFFFFFFFF;
    }

    cm_node_t* popped_node = stack->top;
    stack->top = stack->top->next;
    stack->nodecount -= 1;

    uint32_t popped_data = popped_node->move;
    free(popped_node);

    return popped_data;
}

void cm_stack_destroy(cm_stack_t* stack) {
    stack->nodecount = 0;

    if (stack->top == NULL) {
        free(stack);
        return;
    }

    cm_node_t* destroyed_node = stack->top;
    cm_node_t* next_node;

    do {
        next_node = destroyed_node->next;
        free(destroyed_node);
        destroyed_node = next_node;
    } while (destroyed_node != NULL);

    free(stack);
}
