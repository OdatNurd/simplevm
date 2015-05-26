#ifndef __OPCODESdotH__
#define __OPCODESdotH__

/***********************************************************************************************************/

/* This represents the various VM Opcodes that can be issued in our program. */
typedef enum
{
    /* Do nothing. */
    NOP=0,

    /* Push the operand onto the stack. */
    PUSH,

    /* Pop the top item from the stack. */
    POP,

    /* Set the register in the operand to the value on the top of the stack. */
    SET,
       
    /* Pop the top two values on the stack, and then push the result of their addition. */
    ADD,

    /* Take two operands (two registers) and add them together, pushing the result to the stack. */
    RADD,

    /* Decrement the register in the first operand. */
    RDEC,

    /* Halt program execution. The second version is used by the interpreter internally to signal that the
     * program provided did not have its own halt statement. */
    HALT,
    IHALT,
} Opcode;

/***********************************************************************************************************/

#endif
