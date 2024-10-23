#include "cm_stack.h"
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    uint32_t first_tested_word = 0xABCD1234;
    uint32_t second_tested_word = 0x12345678;
    uint32_t third_tested_word = 0xDEADBEEF;
    uint32_t fourth_tested_word = 0x567890AB;

    cm_stack_t* tested_stack = cm_stack_init();
    assert(tested_stack != NULL); 
    assert(tested_stack->top == NULL);
    assert(tested_stack->nodecount == 0);

    assert(cm_stack_pop(tested_stack) == 0xFFFFFFFF);

    assert(cm_stack_pop(tested_stack) == 0xFFFFFFFF);

    cm_stack_push(tested_stack, first_tested_word);
    assert(tested_stack->top->move == first_tested_word);
    assert(tested_stack->nodecount == 1);

    assert(cm_stack_pop(tested_stack) == first_tested_word);

    cm_stack_push(tested_stack, second_tested_word);
    assert(tested_stack->top->move == second_tested_word);

    cm_stack_push(tested_stack, third_tested_word);
    assert(tested_stack->top->move == third_tested_word);

    cm_stack_push(tested_stack, fourth_tested_word); 
    assert(tested_stack->top->move == fourth_tested_word);

    assert(cm_stack_pop(tested_stack) == fourth_tested_word);
    assert(cm_stack_pop(tested_stack) == third_tested_word);
    assert(cm_stack_pop(tested_stack) == second_tested_word);

    cm_stack_push(tested_stack, first_tested_word);
    cm_stack_push(tested_stack, second_tested_word);
    cm_stack_push(tested_stack, third_tested_word);
    cm_stack_push(tested_stack, fourth_tested_word); 

    cm_stack_destroy(tested_stack);

    return 0;
}
