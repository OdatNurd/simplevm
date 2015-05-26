/***********************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "vm.h"

/***********************************************************************************************************/

/* Convert an opcode into a textual name. */
static const char *opcode_name (Opcode opcode)
{
    switch (opcode)
    {
        case NOP:   return "NOP";
        case PUSH:  return "PUSH";
        case POP:   return "POP";
        case SET:   return "SET";
        case ADD:   return "ADD";
        case RADD:  return "RADD";
        case HALT:  return "HALT";
        case IHALT: return "IHALT";
    }

    /* This isn't a default case so that we can determine when we forgot to modify this switch. */
    return "???";
}

/***********************************************************************************************************/

/* Convert a register into a textual name. */
static const char *register_name (Register reg)
{
    switch (reg)
    {
        case REG_A: return "REG_A";
        case REG_B: return "REG_B";
        case REG_C: return "REG_C";
        case REG_D: return "REG_D";
        case REG_E: return "REG_E";
        case REG_F: return "REG_F";

        /* Do nothing here, so we fall through and trigger on the default below. */
        case REGISTER_COUNT:
                    break;
    }

    /* This isn't a default case so that we can determine when we forgot to modify this switch. */
    return "???";
}

/***********************************************************************************************************/

/* Convert the error reason from an IHALT instruction into a human readable string. The opcode paramter
 * provided is only valid in cases where decode_instruction() detected an error that requires the offending
 * opcode to be used in the error and for which it remembers to set it. Otherwise it's probably NOP. 
 *
 * This *might* use static storage, so make a copy of the return value if you want it to remain valid between
 * calls. */
static const char *ihalt_error_reason (IHALT_Reason errorReason, Opcode opcode)
{
    /* Only used for errors that need an opcode. */
    static char buffer[256];

    /* Handle the different IHalt reasons. */
    switch (errorReason)
    {
        /* We don't know the reason for the error. */
        case IHALT_UNKNOWN: 
            return "Unknown error (maybe fix decode_instruction() so this doesn't happen?)";

        /* An IHALT instruction was explicitly found in the bytecode, which is not allowed because they are
         * our error mechanism. */
        case IHALT_IHALT_EXPLICIT:
            return "IHALT instructions are not allowed in the bytecode stream";

        /* The last instruction should be a HALT opcode which terminates the program. */
        case IHALT_MISSING_OPCODE:
            return "Bytecode terminates without a HALT opcode";

        /* The bytecode stream ends with an opcode that needs more parameters than are available. */
        case IHALT_MISSING_OPCODE_PARAMETER:
            snprintf (buffer, sizeof (buffer), "Opcode (%s) requires more parameters than are available in the bytecode stream", opcode_name (opcode));
            return buffer;
    }

    return "So broken I don't even know that the error is an unknown error!";
}

/***********************************************************************************************************/

/* Obtain the number of operands an opcode expects. */
static int opcode_operand_count (Opcode opcode)
{
    switch (opcode)
    {
        /* Need a value to push. */
        case PUSH: 
            return 1;

        /* Need the register to set. */
        case SET:  
            return 1;

        /* Need two registers to add. */
        case RADD:
            return 2;

        /* These operate on the stack or otherwise do not require parameters. */
        case NOP:
        case POP:  
        case ADD:  
        case HALT: 
            return 0;

        /* The IHALT instruction actualy takes 1 or more arguments, but it's not allowed to appear in user
         * programs, so as far as this is concerned, it takes none. This allows for us to detect that it
         * exists in the program stream and flag it as an error without having to fiddle with worrying if it
         * has enough parameters. */
        case IHALT:
            return 0;
    }

    /* Because the compiler is kind of stupid. */
    return 0;
}

/***********************************************************************************************************/

/* Decode the next instruction in the program stream of the given context into the buffer provided. 
 *
 * The instruction comes out as an internal halt (IHALT) if there is an error fetching the opcode or its
 * paramters. */
static void decode_instruction (VMContext *context, Instruction *instruction)
{
    Opcode opcode;
    int i,count;

    /* Default the instruction to being an error halt, which takes 1 parameter. The paramter is filled with
     * the reason for the error, which at this point is unknown. */
    instruction->opcode        = IHALT;
    instruction->parameters[0] = IHALT_UNKNOWN;
    instruction->pCount        = 1;

    /* Leave if there is not an opcode left. */
    if (context->ip >= context->pSize)
    {
        /* Swap the error reason. */
        instruction->parameters[0] = IHALT_MISSING_OPCODE;
        return;
    }

    /* Fetch this opcode and determine how many operands it requires. */
    opcode = context->program[context->ip];
    count  = opcode_operand_count (opcode);

    /* If this is an IHALT instruction, that's bad. */
    if (opcode == IHALT)
    {
        instruction->parameters[0] = IHALT_IHALT_EXPLICIT;
        return;
    }

    /* Leave if there are not enough extra slots in the program to fulfill all of the arguments of this
     * particular opcode. */
    if (context->ip + opcode_operand_count (opcode) >= context->pSize)
    {
        /* In this case, we need to change the error message and also add in the offending opcode, which
         * requires altering the paramter count. */
        instruction->parameters[0] = IHALT_MISSING_OPCODE_PARAMETER;
        instruction->parameters[1] = opcode;
        instruction->pCount++;
        return;
    }

    /* We're all good, so copy over. */
    instruction->opcode = opcode;
    instruction->pCount = count;

    /* Copy the required paramters over. We need to add 1 to the ip to skip over the instruction. */
    for (i = 0 ; i < count ; i++)
        instruction->parameters[i] = context->program[context->ip + i + 1];
}

/***********************************************************************************************************/

/* Output a trace of the instruction that the VM is currently sitting at. */
static void vmtrace (VMContext *context, Instruction *instruction)
{
    int i;

    /* Detect errors in the user program. These are developer errors because they indicate that the bytecode
     * stream is observably broken. */
    if (instruction->opcode == IHALT)
    {
        fprintf (stderr, ">> *** << Invalid program detected\n");
        fprintf (stderr, ">> *** << %s\n", ihalt_error_reason (instruction->parameters[0], instruction->parameters[1]));
        return;
    }

    fprintf (stderr, ">>> %s", opcode_name (instruction->opcode));
    for (i = 0 ; i < instruction->pCount ; i++)
        fprintf (stderr, " %d ", instruction->parameters[i]);
    fprintf (stderr, "\n");
}

/***********************************************************************************************************/

/* Evaluate (execute) a single VM instruction in the provided context. */
static void evaluate (VMContext *context, Instruction *instruction)
{
    /* The new IP is going to be the current IP, plus 1 to skip over this instruction, plus however many
     * parameters this instruction took. */
    int new_ip = context->ip + instruction->pCount + 1;

    /* Handle based on opcode. */
    switch (instruction->opcode)
    {
        /* Do nothing. */
        case NOP:
            break;

        /* Push the next byte onto the stack. We pre-increment the instruction pointer since it's currently
         * sitting on the push instruction. */
        case PUSH:
            ctx_stack_push (context, instruction->parameters[0]);
            break;

        /* Pop the top value from the stack. This will also display the value that was popped. */
        case POP:
            {
                int result = ctx_stack_pop (context);
                fprintf (stderr, "<<POP>> %d\n", result);
                break;
            }

        /* Not currently handled. */
        case SET:
            {
                int dReg = instruction->parameters[0];
                int value = ctx_stack_pop (context);
                context->registers[dReg] = value;
                fprintf (stderr, "<<SET %s>> %d\n", register_name (dReg), value);
            }
            break;

        /* Pop two values from the stack, add them together, and then push the result back. */
        case ADD:
            ctx_stack_push (context, ctx_stack_pop (context) + ctx_stack_pop (context));
            break;

        /* Get the values of the two registers used as operands and push the result of adding them. */
        case RADD:
            {
                int reg1 = instruction->parameters[0];
                int reg2 = instruction->parameters[1];
                ctx_stack_push (context, context->registers[reg1] + context->registers[reg2]);
            }
            break;

        /* The HALT instruction sets the HALT flag on this context, telling the interpreter that all
         * operations are now complete. */
        case HALT:
        case IHALT:
            context->halted = 1;
            break;
    }

    /* Update the IP on return. */
    context->ip = new_ip;
      
}

/***********************************************************************************************************/

/* Run the program in the provided context.  */
void vm_interpret (VMContext *context)
{
    Instruction instruction;

    /* Keep looping until we determine that we are done running. */
    while (context->halted == 0)
    {
        /* Decode the next instruction and set what the new IP will be to execute the next instruction. */
        decode_instruction (context, &instruction);
        vmtrace (context, &instruction);

        /* Execute the instruction now. */
        evaluate (context, &instruction);
    }
}

/***********************************************************************************************************/

