#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* This specifies how large of a stack the VM is allowed to have. This is specified in stack entries. */
#define CONTEXT_STACK_SIZE 256

/* This specifies the number of parameters (at maximum) an opcode can have. */
#define MAX_OPCODE_PARAMS 5

/* This represents the various VM Opcodes that can be issued in our program. */
typedef enum
{
    /* Do nothing. */
    NOP=0,

    /* Push the operand onto the stack. */
    PUSH,

    /* Pop the top item from the stack. */
    POP,

    /* Set a register. */
    SET,
       
    /* Add values. */
    ADD,

    /* Halt program execution. The second version is used by the interpreter internally to signal that the
     * program provided did not have its own halt statement. */
    HALT,
    IHALT,
} Opcode;

/* The IHALT instruction is a special internal HALT instruction that terminals the program but also has a
 * reason why. 
 *
 * The IHALT always has at least 1 parameter, which is the reason for the error and is one of the values in
 * this enumeration. Depending on the error, other parameters may exist. For example, if the error is that an
 * opcode was missing a parameter, the opcode in question is carried in another parameter. */
typedef enum
{
    /* A mysterious error of mystery has occurred. */
    IHALT_UNKNOWN,

    /* The program code was found to contain an IHALT instruction, which is not allowed. */
    IHALT_IHALT_EXPLICIT,

    /* The HALT opcode was missing at the end of the operation. */
    IHALT_MISSING_OPCODE,

    /* The opcode decoded was missing some number of parameters. The opcode in question is in the second
     * paramter to the IHALT opcode. */
    IHALT_MISSING_OPCODE_PARAMETER,
} IHALT_Reason;

/* This structure represents a decoded instruction from the program stream. */
typedef struct
{
    /* The opcode that indicates what the instruction is supposed to do. */
    Opcode opcode;

    /* The parameters to the opcode, and a description of how many of the slots are used. */
    int parameters[MAX_OPCODE_PARAMS];
    int pCount;
} Instruction;

typedef enum
{
    /* General purpose registers. */
    REG_A,
    REG_B,
    REG_C,
    REG_D,
    REG_E,
    REG_F,

    /* The total number of registers. */
    REGISTER_COUNT,
} Register;

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

/* Convert an opcode into a textual name. */
const char *opcode_name (Opcode opcode)
{
    switch (opcode)
    {
        case NOP:   return "NOP";
        case PUSH:  return "PUSH";
        case POP:   return "POP";
        case SET:   return "SET";
        case ADD:   return "ADD";
        case HALT:  return "HALT";
        case IHALT: return "IHALT";
    }

    /* This isn't a default case so that we can determine when we forgot to modify this switch. */
    return "???";
}

/* Convert the error reason from an IHALT instruction into a human readable string. The opcode paramter
 * provided is only valid in cases where decode_instruction() detected an error that requires the offending
 * opcode to be used in the error and for which it remembers to set it. Otherwise it's probably NOP. 
 *
 * This *might* use static storage, so make a copy of the return value if you want it to remain valid between
 * calls. */
const char *ihalt_error_reason (IHALT_Reason errorReason, Opcode opcode)
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

/* Obtain the number of operands an opcode expects. */
int opcode_operand_count (Opcode opcode)
{
    switch (opcode)
    {
        /* Need a value to push. */
        case PUSH: 
            return 1;

        /* These operate on the stack or otherwise do not require parameters. */
        case NOP:
        case POP:  
        case SET:  
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

/* Decode the next instruction in the program stream of the given context into the buffer provided. 
 *
 * The instruction comes out as an internal halt (IHALT) if there is an error fetching the opcode or its
 * paramters. */
void decode_instruction (VMContext *context, Instruction *instruction)
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

/* Output a trace of the instruction that the VM is currently sitting at. */
void vmtrace (VMContext *context, Instruction *instruction)
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

/* Push a value onto the stack of the provided VM context. */
void context_stack_push (VMContext *context, int value)
{
    /* Increment the stack pointer and then store the value. The stack pointer is -1 when empty. */
    context->stack[++context->sp] = value;
}

/* Pop a value from the stack. */
int context_stack_pop (VMContext *context)
{
    /* Get the value at the current stack position, then decrement the stack pointer. It becomes -1 when the
     * stack is empty. */
    return context->stack[context->sp--];
}

/* Evaluate (execute) a single VM instruction in the provided context. */
void evaluate (VMContext *context, Instruction *instruction)
{
    switch (instruction->opcode)
    {
        /* Do nothing. */
        case NOP:
            break;

        /* Push the next byte onto the stack. We pre-increment the instruction pointer since it's currently
         * sitting on the push instruction. */
        case PUSH:
            context_stack_push (context, instruction->parameters[0]);
            break;

        /* Pop the top value from the stack. This will also display the value that was popped. */
        case POP:
            {
                int result = context_stack_pop (context);
                fprintf (stderr, "<<POP>> %d\n", result);
                break;
            }

        /* Not currently handled. */
        case SET:
            break;

        /* Pop two values from the stack, add them together, and then push the result back. */
        case ADD:
            context_stack_push (context, context_stack_pop (context) + context_stack_pop (context));
            break;

        /* The HALT instruction sets the HALT flag on this context, telling the interpreter that all
         * operations are now complete. */
        case HALT:
        case IHALT:
            context->halted = 1;
            break;
    }
}

/* Run the program in the provided context.  */
void interpret (VMContext *context)
{
    Instruction instruction;

    /* Keep looping until we determine that we are done running. */
    while (context->halted == 0)
    {
        /* Decode the next instruction and output it. */
        decode_instruction (context, &instruction);
        vmtrace (context, &instruction);

        /* Run the instruction now. */
        evaluate (context, &instruction);

        /* Advance the instruction pointer now. */
        context->ip += instruction.pCount + 1;
    }
}

/* Initialize a VM context to run the provided program. */
void init_context (VMContext *context, int *program, int programLength)
{
    /* Zero it. */
    memset (context, 0, sizeof (VMContext));

    /* Set up the program. */
    context->program = program;
    context->pSize   = programLength;
    context->ip      = 0;

    /* The stack starts empty. */
    context->sp = -1;

    /* Not initially halted. */
    context->halted = 0;
}

int main (int argc, char **argv)
{
    VMContext context;

    /* Set up a program context and then run it. */
    init_context (&context, program, sizeof (program) / sizeof (int));
    interpret (&context);
 
    return 0;
}


