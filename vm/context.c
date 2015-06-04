/***********************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vm.h"

/***********************************************************************************************************/

/* Initialize a VM context to run the provided program, which is assumed to be of the given length.
 *
 * As a convenience, the initialized context is returned back by the call. */
VMContext *ctx_init (VMContext *context, int *program, int programLength)
{
    /* Zero it. */
    memset (context, 0, sizeof (VMContext));

    /* Set up the program. */
    context->program = program;
    context->pSize   = programLength;
    context->ip      = 0;

    /* The stack starts empty. */
    context->sp = -1;

    /* Not initially halted. */
    context->halted = 0;

    /* Return the initialized context back. */
    return context;
}

/***********************************************************************************************************/

/* Push a value onto the stack of the provided VM context.
 *
 * If the stack is not full, then the stack overflow bit is cleared and the function returns after pushing
 * the item to the top of the stack and updating sp. If the stack is full, the stack overflow bit is set
 * and the function returns without doing anything else. */
void ctx_stack_push (VMContext *context, int value)
{
    /* Make sure there is room in the stack. */
    if (context->sp == CONTEXT_STACK_SIZE - 1)
    {
        context->vmFlags.stackOverflow = 1;
        return;
    }

    /* Increment the stack pointer and then store the value. The stack pointer is -1 when empty. */
    context->stack[++context->sp] = value;
    context->vmFlags.stackOverflow = 0;
}

/***********************************************************************************************************/

/* Pop a value from the stack and return it.
 *
 * If the stack is not empty, then the stack underflow bit is cleared and the function returns after
 * removing the item from the top of the stack and updating sp. If the stack is empty, the stack underflow
 * bit is set and the function returns 0. */
int ctx_stack_pop (VMContext *context)
{
    /* Make sure there is something on the stack. */
    if (context->sp == -1)
    {
        context->vmFlags.stackUnderflow = 1;
        return 0;
    }

    /* Get the value at the current stack position, then decrement the stack pointer. It becomes -1 when the
     * stack is empty. */
    context->vmFlags.stackUnderflow = 0;
    return context->stack[context->sp--];
}

/***********************************************************************************************************/

/* Peek at the item at the top of the stack without popping it.
 *
 * This function in all regards is identical to ctx_stack_pop() except that it does not modify sp. */
int ctx_stack_peek (VMContext *context)
{
    /* Make sure there is something on the stack. */
    if (context->sp == -1)
    {
        context->vmFlags.stackUnderflow = 1;
        return 0;
    }

    /* Get the value at the current stack position. */
    context->vmFlags.stackUnderflow = 0;
    return context->stack[context->sp];
}

/***********************************************************************************************************/

