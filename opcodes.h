#ifndef __OPCODESdotH__
#define __OPCODESdotH__

/***********************************************************************************************************/

/* Programs in the VM are made up of one or more Opcodes which describe the action to take. Each opcode can
 * take 0 or more arguments (which appear after it in the bytecode stream).
 *
 * A valid program should terminate with a HALT opcode; the VM will issue an automatic IHALT opcode if it
 * discovers an inconsistency in the bytecode, such as a missing HALT opcode, unknown OPCODE, etc. */
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

    /* Jump instructions. These take two parameters, the first being a register and the second being an IP
     * offset value (positive or negative). The register is compared to the item at the top of the stack
     * (which is not consumed/popped), and if the comparison is true, the IP is jumped by the offset provided
     * in the second operand, which can be negative to jump upwards. 
     *
     * The offset is applied to the IP while it points at the jump instruction itself (so an offset of 0 would
     * make it sit in an infinite loop comparing, for example). */
    RJNE,

    /* Halt program execution. This indicates a normal halt. */
    HALT,

    /* Halt program execution. This is used by the interpreter internally to signal that the program
     * provided did not have its own halt statement. */
    IHALT,
} Opcode;

/***********************************************************************************************************/

#endif
