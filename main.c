/***********************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "vm.h"

/***********************************************************************************************************/

/* A simple program. */
int program[] = {
    /* Set up the A and B  registers. */
    PUSH, 3,
    SET, REG_A,
    RDEC, REG_A,
    PUSH, 3,
    SET, REG_B,

    /* Add them. Result is left on the stack. */
    RADD, REG_A, REG_B,

    /* Push one more values. */
    PUSH, 6,

    /* Add the two items on the stack. */
    ADD,

    /* Pop the result and then halt. */
    POP,
    HALT
};

/***********************************************************************************************************/

/* Entry point. */
int main (int argc, char **argv)
{
    /* Our interpreter context. */
    VMContext context;

    /* Set up a program context and then run it. */
    ctx_init (&context, program, sizeof (program) / sizeof (int));
    vm_interpret (&context);
 
    return 0;
}

/***********************************************************************************************************/

