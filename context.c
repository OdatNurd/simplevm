/***********************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "context.h"

/***********************************************************************************************************/

/* This specifies how large of a stack the VM is allowed to have. This is specified in stack entries. */
#define CONTEXT_STACK_SIZE 256

/***********************************************************************************************************/

/* Initialize a VM context to run the provided program. */
void ctx_init (VMContext *context, int *program, int programLength)
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
}

/***********************************************************************************************************/


/* Push a value onto the stack of the provided VM context. */
void ctx_stack_push (VMContext *context, int value)
{
    /* Increment the stack pointer and then store the value. The stack pointer is -1 when empty. */
    context->stack[++context->sp] = value;
}

/***********************************************************************************************************/

/* Pop a value from the stack. */
int ctx_stack_pop (VMContext *context)
{
    /* Get the value at the current stack position, then decrement the stack pointer. It becomes -1 when the
     * stack is empty. */
    return context->stack[context->sp--];
}

/***********************************************************************************************************/

