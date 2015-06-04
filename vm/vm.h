#ifndef __VMdotH__
#define __VMdotH__

/***********************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "registers.h"
#include "opcodes.h"
#include "context.h"

/***********************************************************************************************************/

/* This specifies the number of parameters (at maximum) an opcode can have. */
#define MAX_OPCODE_PARAMS 5

/***********************************************************************************************************/

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

/***********************************************************************************************************/

/* Run the program in the provided context.  */
void vm_interpret (VMContext *context);

/***********************************************************************************************************/

#endif
