#ifndef __CONTEXTdotH__
#define __CONTEXTdotH__

/***********************************************************************************************************/

/* This specifies how large of a stack the VM is allowed to have. This is specified in stack entries. */
#define CONTEXT_STACK_SIZE 256

/***********************************************************************************************************/

/* This structure represents a VM context, which is what a program runs in in the VM. All global state for an
 * interpreter is kept here so that there can be multiple interpreters at once if desired. */
typedef struct
{
    /* The program being executed. This is a set of integers that are the opcodes and their operands. */
    int *program;

    /* How big the program is, in integers (i.e. the size of the program array). */
    int pSize;

    /* True if a halt opcode has been encountered in this program, false otherwise. */
    int halted;

    /* The instruction pointer; this points to the instruction to be executed in the program. */
    int ip;

    /* The operations stack for this context, and the index of the next slot. */
    int stack[CONTEXT_STACK_SIZE];

    /* The stack pointer for the stack in this context. The index -1 indicates that the stack is empty;
     * otherwise the value here is the index of the item at the top of the stack. */
    int sp;

    /* This value is set to 1 whenever a stack push operation failed because the stack was already full, or
     * when a stack pop failed because the stack was empty. When a stack operation completes successfully,
     * this gets set to 0.
     *
     * The value is only changed when a stack operation is performed; at other times the value remains
     * as-is. */
    int stackOverUnder;

    /* The registers for this particular context. */
    int registers[REGISTER_COUNT];
} VMContext;

/***********************************************************************************************************/

/* Initialize a VM context to run the provided program, which is assumed to be of the given length.
 *
 * As a convenience, the initialized context is returned back by the call. */
VMContext *ctx_init (VMContext *context, int *program, int programLength);

/* Push a value onto the stack of the provided VM context.
 *
 * If the stack is not full, then the stack overflow bit is cleared and the function returns after pushing
 * the item to the top of the stack and updating sp. If the stack is full, the stack overflow bit is set
 * and the function returns without doing anything else. */
void ctx_stack_push (VMContext *context, int value);

/* Pop a value from the stack and return it.
 *
 * If the stack is not empty, then the stack underflow bit is cleared and the function returns after
 * removing the item from the top of the stack and updating sp. If the stack is empty, the stack underflow
 * bit is set and the function returns 0. */
int ctx_stack_pop (VMContext *context);

/* Peek at the item at the top of the stack without popping it. Returns what ctx_stack_pop() would return. */
int ctx_stack_peek (VMContext *context);

/***********************************************************************************************************/

#endif
