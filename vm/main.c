/***********************************************************************************************************/

#include <stdlib.h>
#include <core/vm.h>

/***********************************************************************************************************/

/* A simple program. */
int program[] = {
//     /* Set up the A and B  registers. */
//     PUSH, 3,
//     SET, REG_A,
//     RDEC, REG_A,
//     PUSH, 3,
//     SET, REG_B,
// 
//     /* Add them. Result is left on the stack. */
//     RADD, REG_A, REG_B,
// 
//     /* Push one more values. */
//     PUSH, 6,
// 
//     /* Add the two items on the stack. Result is left on the stack. */
//     ADD,
// 
//     /* Pop the result (also prints it). */
//     POP,

    /* Set register F to 10. */
    PUSH, 10,
    SET, REG_F,

    /* Push the value 0 to the stack. */
    PUSH, 0,

    /* Decrement register F. */
    RDEC, REG_F,

    /* Jump backwards 2 if register F is not equal to the item at the top of the stack. */
    RJNE, REG_F, -2,

    /* All done now. */
    HALT
};

/***********************************************************************************************************/

/* Entry point. */
int main (int argc, char **argv)
{
    /* Our interpreter context. */
    VMContext context;

    /* Set up a program context and then run it. */
    vm_interpret (ctx_init (&context, program, sizeof (program) / sizeof (int)));
 
    return 0;
}

/***********************************************************************************************************/

