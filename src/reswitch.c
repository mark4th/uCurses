// did i say how much i hate the c switch statememt?
// -----------------------------------------------------------------------

#include "h/uCurses.h"

// -----------------------------------------------------------------------

// create an array of switch_t structures.  where each element of the
// structure contains a option and a function vector.  for example
// see parse.c

int re_switch(const switch_t *s, size_t size, uint32_t option)
{
    int result = -1;

    while((size != 0) && (option != s->option))
    {
        s++;
        size--;
    }
    if((size != 0) && (option == s->option))
    {
        (s->vector)();
        result = 0;
    }

    return result;
}

// =======================================================================
