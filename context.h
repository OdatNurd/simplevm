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
    /* The program being executed. This is a set of integers that are the opcodes and their operands. We also
     * keep a note of how big the list is. */
    int *program;
    int pSize;

    /* True if a halt opcode has been encountered in this program, false otherwise. */
    int halted;

    /* The instruction pointer; this points to the instruction to be executed in the program. */
    int ip;

    /* The operations stack for this context, and the index of the next slot. The index is -1 when the stack
     * is empty. */
    int stack[CONTEXT_STACK_SIZE];
    int sp;

    /* The registerss for this particular context. */
    int registers[REGISTER_COUNT];
} VMContext;

/***********************************************************************************************************/

/* Initialize a VM context to run the provided program. */
void ctx_init (VMContext *context, int *program, int programLength);

/* Push a value onto the stack of the provided VM context. */
void ctx_stack_push (VMContext *context, int value);

/* Pop a value from the stack. */
int ctx_stack_pop (VMContext *context);

/* Peek at the item at the top of the stack without popping it. Returns what ctx_stack_pop() would return. */
int ctx_stack_peek (VMContext *context);

/***********************************************************************************************************/

#endif
