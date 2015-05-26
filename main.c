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
    PUSH, 5,
    SET, REG_A,
    PUSH, 6,
    SET, REG_B,

    /* Push two values. */
    PUSH, 5,
    PUSH, 6,

    /* Add them. */
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

