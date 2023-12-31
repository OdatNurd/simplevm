/***********************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "vm.h"
#include "context.h"

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
        case RDEC:  return "RDEC";
        case RJNE:  return "RJNE";
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

/* Convert the error reason from an IHALT instruction into a human readable string. The opcode parameter
 * provided is only valid in cases where decode_instruction() detected an error that requires the offending
 * opcode to be used in the error and for which it remembers to set it. Otherwise it's probably NOP. 
 *
 * This *might* use static storage, so make a copy of the return value if you want it to remain valid between
 * calls. */
static char ihalt_error_buffer[256];
static const char *ihalt_error_reason (IHALT_Reason errorReason, Opcode opcode)
{
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
            snprintf (ihalt_error_buffer, sizeof (ihalt_error_buffer), "Opcode (%s) requires more parameters than are available in the bytecode stream", opcode_name (opcode));
            return ihalt_error_buffer;

        case IHALT_STACK_OVERFLOW:
            return "Stack overflow";

        case IHALT_STACK_UNDERFLOW:
            return "Stack underflow";
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
              
        /* Needs the register to decrement. */
        case RDEC:
            return 1;

        /* Requires a register and a jump offset. */
        case RJNE:
            return 2;

        /* These operate on the stack or otherwise do not require parameters. */
        case NOP:
        case POP:  
        case ADD:  
        case HALT: 
            return 0;

        /* The IHALT instruction actually takes 1 or more arguments, but it's not allowed to appear in user
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

/* Obtain the operand mask for an opcode. This is a simple string that contains one character for each of the
 * operands, where each character is laid out as follows:
 *     i: an integer number
 *     r: a register
 *
 * This is used by the trace functionality to display operands properly. */
static const char *opcode_operand_mask (Opcode opcode)
{
    switch (opcode)
    {
        /* Need a value to push. */
        case PUSH: 
            return "i";

        /* Need the register to set. */
        case SET:  
            return "r";

        /* Need two registers to add. */
        case RADD:
            return "rr";

        /* Needs the register to decrement. */
        case RDEC:
            return "r";

        /* Requires a register and a jump offset. */
        case RJNE:
            return "ri";

        /* These operate on the stack or otherwise do not require parameters. */
        case NOP:
        case POP:  
        case ADD:  
        case HALT: 
            return "";

        /* The IHALT instruction actually takes 1 or more arguments, but it's not allowed to appear in user
         * programs, so as far as this is concerned, it takes none. This allows for us to detect that it
         * exists in the program stream and flag it as an error without having to fiddle with worrying if it
         * has enough parameters. */
        case IHALT:
            return "";
    }

    /* Because the compiler is kind of stupid. */
    return "";
}

/***********************************************************************************************************/

/* Initialize an IHALT instruction. The reason for the halt will be the one provided. All other parameters
 * are the parameters that are required for the specific error reason. It's up to the caller to pass in the
 * required values in the required order. */
static void init_ihalt_instruction (Instruction *instruction, IHALT_Reason reason, int params, ...)
{
    va_list args;

    /* Minimum used parameters is 1, for the reason. */
    int usedParams = 1;

    /* Set up the opcode and the single known parameter, which is the reason for the halt. */
    instruction->opcode = IHALT;
    instruction->parameters[0] = reason;

    /* Based on the reason, add in the extra parameters. */
    va_start(args, params);
    switch (reason)
    {
        /* The extra parameter is the first parameter, which we don't need special gymnastics to get. */
        case IHALT_MISSING_OPCODE_PARAMETER:
            instruction->parameters[1] = params;
            usedParams++;
            break;

        /* va_arg(args, int); */

        /* All other reasons require no parameters. */
        default:
            break;
    }
    va_end(args);

    instruction->pCount = usedParams;
}

/***********************************************************************************************************/

/* Decode the next instruction in the program stream of the given context into the buffer provided. 
 *
 * The instruction comes out as an internal halt (IHALT) if there is an error fetching the opcode or its
 * parameters. */
static void decode_instruction (VMContext *context, Instruction *instruction)
{
    Opcode opcode;
    int i, count;

    /* Default the instruction to being an unknown error halt. */
    init_ihalt_instruction (instruction, IHALT_UNKNOWN, 0);

    /* Leave if there is not an opcode left. */
    if (context->ip >= context->pSize)
    {
        /* Swap the error reason. */
        instruction->parameters[0] = IHALT_MISSING_OPCODE;
        return;
    }

    /* Fetch this opcode and determine how many operands it requires. */
    opcode = (Opcode) context->program[context->ip];
    count = opcode_operand_count (opcode);

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
         * requires altering the parameter count. */
        instruction->parameters[0] = IHALT_MISSING_OPCODE_PARAMETER;
        instruction->parameters[1] = opcode;
        instruction->pCount++;
        return;
    }

    /* We're all good, so copy over. */
    instruction->opcode = opcode;
    instruction->pCount = count;

    /* Copy the required parameters over. We need to add 1 to the ip to skip over the instruction. */
    for (i = 0 ; i < count ; i++)
        instruction->parameters[i] = context->program[context->ip + i + 1];
}

/***********************************************************************************************************/

/* Assume that the instruction passed in is an IHALT instruction and display the reason that the halt is
 * happening. Once this is done, the VM context is marked as being halted. */
static void vm_ihalt (VMContext *context, Instruction *instruction)
{
    /* The first parameter is always the error reason. */
    IHALT_Reason errorReason = (IHALT_Reason) instruction->parameters[0];

    /* Currently the only thing that uses the second parameter is the error about a missing opcode. */
    Opcode missingOpcode = (Opcode) instruction->parameters[1];

    /* Display the message now. */
    fprintf (stderr, ">> *** << Invalid program detected\n");
    fprintf (stderr, ">> *** << %s\n", ihalt_error_reason (errorReason, missingOpcode));

    /* No more operations on this context now. */
    context->halted = 1;
}

/***********************************************************************************************************/

/* Output a trace of the instruction that the VM is currently sitting at. */
static void vm_trace (VMContext *context, Instruction *instruction)
{
    int i;
    const char *mask;

    /* Detect errors in the user program. These are developer errors because they indicate that the bytecode
     * stream is observably broken. */
    if (instruction->opcode == IHALT)
    {
        vm_ihalt (context, instruction);
        return;
    }

    /* This string mask tells us what each of the operands is, so that we can display it properly. */
    mask = opcode_operand_mask (instruction->opcode);

    fprintf (stderr, ">>> %s", opcode_name (instruction->opcode));
    for (i = 0 ; i < instruction->pCount ; i++)
    {
        if (mask[i] == 'r')
        {
            int reg = instruction->parameters[i];
            fprintf (stderr, " %s (%d) ", register_name ((Register) reg), context->registers[reg]);
        }
        else
            fprintf (stderr, " %d ", instruction->parameters[i]);
    }
    fprintf (stderr, "\n");
}

/***********************************************************************************************************/

/* Check the stack overflow/undeflow bits in the provided context. If either is set, the appropriate error
 * message is displayed, the VM is halted, and 1 is returned.
 *
 * If all is OK, 0 is returned instead. */
static int check_stack (VMContext *context)
{
    Instruction iHalt;

    if (context->vmFlags.stackOverflow)
        init_ihalt_instruction (&iHalt, IHALT_STACK_OVERFLOW, 0);
    else if (context->vmFlags.stackUnderflow)
        init_ihalt_instruction (&iHalt, IHALT_STACK_UNDERFLOW, 0);
    else
        return 0;

    vm_ihalt (context, &iHalt);
    return 1;
}

/***********************************************************************************************************/

/* Evaluate (execute) a single VM instruction in the provided context. */
static void evaluate (VMContext *context, Instruction *instruction)
{
    /* Used to halt the VM if we encounter an error. */
    Instruction ihalt;

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
            if (check_stack (context))
                break;
            break;

        /* Pop the top value from the stack. This will also display the value that was popped. */
        case POP:
            {
                int result = ctx_stack_pop (context);
                if (check_stack (context))
                    break;

                fprintf (stderr, "<<POP>> %d\n", result);
                break;
            }

        /* Set a register from the stack. */
        case SET:
            {
                int dReg = instruction->parameters[0];
                int value = ctx_stack_pop (context);
                if (check_stack (context))
                    break;

                context->registers[dReg] = value;
                fprintf (stderr, "<<SET %s>> %d\n", register_name ((Register) dReg), value);
            }
            break;

        /* Pop two values from the stack, add them together, and then push the result back. */
        case ADD:
        {
            int p1, p2;
            p1 = ctx_stack_pop (context);
            if (check_stack (context))
                break;

            p2 = ctx_stack_pop (context);
            if (check_stack (context))
                break;

            ctx_stack_push (context, p1 + p2);
        }
            break;

        /* Get the value of the register provided in the first operand and subtract one from it. */
        case RDEC:
            {
                int reg = instruction->parameters[0];
                context->registers[reg]--;
            }
            break;

        /* Get the values of the two registers used as operands and push the result of adding them. */
        case RADD:
            {
                int reg1 = instruction->parameters[0];
                int reg2 = instruction->parameters[1];
                ctx_stack_push (context, context->registers[reg1] + context->registers[reg2]);
                if (check_stack (context))
                    break;
            }
            break;

        /* If the register is not equal to the item at the top of the stack, jump the IP by the offset
         * provided, which can be negative. */
        case RJNE:
            {
                int reg = instruction->parameters[0];
                int ofs = instruction->parameters[1];
                int val = ctx_stack_peek (context);
                if (check_stack (context))
                    break;

                if (context->registers[reg] != val)
                    new_ip = context->ip + ofs;
            }
            break;

        /* The HALT instruction sets the HALT flag on this context, telling the interpreter that all
         * operations are now complete. */
        case HALT:
        case IHALT:
            context->halted = 1;
            break;
    }

    /* Update the IP on return, unless we're halted now. */
    if (context->halted == 0)
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
        vm_trace (context, &instruction);

        /* Execute the instruction now. */
        evaluate (context, &instruction);
    }
}

/***********************************************************************************************************/

