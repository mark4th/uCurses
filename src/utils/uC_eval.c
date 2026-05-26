// evaluate a string and return a number using any radix
// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>

#include "uCurses.h"

// -----------------------------------------------------------------------

static uint8_t digit(uint8_t c, uint8_t radix, uint8_t *result)
{
    uint8_t d;

    if      (c >= '0' && c <= '9') { d = c - '0'; }
    else if (c >= 'A' && c <= 'Z') { d = c - 'A' + 10; }
    else if (c >= 'a' && c <= 'z') { d = c - 'a' + 10; }
    else                            { return 0; }

    if (d < radix) { *result = d; return 1; }

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
