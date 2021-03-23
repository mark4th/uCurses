// did i say how much i hate the c switch statememt?
// -----------------------------------------------------------------------

#include "h/tui.h"

// -----------------------------------------------------------------------

// create an array of switch_t structures.  where each element of the
// structure contains a option and a function vector.  for example
// see parse.c

void re_switch(const switch_t *s, int n, char c1)
{
    while((n != 0) && c1 != s->option)
    {
       s++; n--;
    }
    if(c1 == s->option)
    {
        (s->vector)();
    }
    else
    {
        // snafu?
    }
}

// =======================================================================
