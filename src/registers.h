#ifndef __REGISSTERSdotH__
#define __REGISSTERSdotH__

/***********************************************************************************************************/

/* The VM supports a series of registers for program use. Some instructions use implicit registers while
 * others take register parameters to determine how/where they operate. */
typedef enum
{
    /* General purpose register A. */
    REG_A,

    /* General purpose register B. */
    REG_B,

    /* General purpose register C. */
    REG_C,

    /* General purpose register D. */
    REG_D,

    /* General purpose register E. */
    REG_E,

    /* General purpose register F. */
    REG_F,

    /* The total number of registers. */
    REGISTER_COUNT,
} Register;

/***********************************************************************************************************/

#endif
