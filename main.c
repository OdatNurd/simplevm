/***********************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "context.h"
#include "vm.h"

/***********************************************************************************************************/

/* A simple program. */
int program[] = {
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
    context_init (&context, program, sizeof (program) / sizeof (int));
    vm_interpret (&context);
 
    return 0;
}

/***********************************************************************************************************/

