// re_switch.c      replacemetn switch construct.  MUCH cleaner sources
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_switch.h"

// -----------------------------------------------------------------------
// did i say how much i *hate* the c switch statememt?

// my objection to the c switch statement is based on the CLUSTER F it
// turns the code into.  If a switch is based on a typedef enum (as it
// should be) the compiler will remind you about any missing items. you
// do not get this protection here so it is up to YOU to ensure you cover
// all the bases.  I am NOT a fan of the compiler playing mommy and
// protecting YOU from YOU.

// create an array of switch_t structures.  where each element of the
// structure contains a option and a function vector.  for example
// see parse.c

API int uC_switch(const uC_switch_t *s, int size, int32_t option)
{
    int result = -1;

    while ((size != 0) && (option != s->option))
    {
        s++;
        size--;
    }
    if ((size != 0) && (option == s->option))
    {
        // this is another deficiency with this method, the vectors
        // are all void f(void)
        (s->vector)();
        result = 0;
    }

    return result;
}

// =======================================================================
