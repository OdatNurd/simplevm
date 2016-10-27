/*****************************************************************************/

#include <Gen/Gen.h>

/*****************************************************************************/

int main (int argc, char **argv)
{
    /* Init Gen before we do anything else. */
    genInit (argc , argv);

    /* Shutdown gen before we exit. */
    genShutdown ();

    return 0;
}

/*****************************************************************************/

/* vim:tw=79 
 */
