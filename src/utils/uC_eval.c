// evaluate a string and return a number using any radix
// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>

#include "uCurses.h"

// -----------------------------------------------------------------------

static uint8_t digit(uint8_t c, uint8_t radix, uint8_t *result)
{
    if (c < '0')
    {
        return 0;
    }

    c -= '0';

    if (c > 17)
    {
        c -= 7;
    }
    else if (c > 9)
    {
        return 0;
    }

    if (c < radix)
    {
        *result = c;
        return 1;
    }

    return 0;
}

// -----------------------------------------------------------------------

API uint8_t eval(uint8_t *s, uint32_t *result, uint8_t radix)
{
    uint32_t val =  0;
    uint8_t n;

    while (*s != '\0')
    {
        if (digit(*s, radix, &n) != 0)
        {
            val *= radix;
            val += n;
            s++;
        }
        else
        {
            return 0;
        }
    }

    *result = val;

    return 1;
}

// =======================================================================
