// json parsing
// -----------------------------------------------------------------------

#include <stdlib.h>
#include <inttypes.h>
#include <aio.h>
#include <string.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

#define FNV_PRIME 0x01000193
#define FNV_BASIS 0x811c9dc5

// -----------------------------------------------------------------------
// FNV-1a on utf8 strings

uint32_t fnv_hash(char *s)
{
    uint32_t hash = FNV_BASIS;
    uint8_t j;

    utf8_encode_t *encode;

    while(*s != '\0')
    {
        encode = utf8_encode(*s);

        for(j = 0; j < encode->len; j++)
        {
            hash *= FNV_PRIME;
            hash ^= *s++;
        }
    }

    return hash;
}
// -----------------------------------------------------------------------
// over engineered (tm)

uint16_t is_keyword(uint32_t *keys, size_t size, uint32_t hash)
{
    uint16_t lo = 0;
    uint16_t hi = size - 1;
    uint16_t z;

    while(lo <= hi)
    {
        z = (lo + hi) >> 1;

        if(hash == keys[z])  { return z + 1; }
        if(hash >  keys[z])  { lo =   z + 1; }
        else                 { hi =   z - 1; }
    }

    return 0;
}

// -----------------------------------------------------------------------
// extract next space delimited token from input data

char *token(char *s)
{
    utf8_encode_t *encode;
    char *p = s;

    // scan past leading white space
    while((*s == 0x20) || (*s == 0x0a))
    {
        s++;
    }

    // scan from current index to next white space, null
    // or end of string
    while((*s != '\0') &&
          (*s != 0x20) && (*s != 0x0a))
    {
        encode = utf8_encode(*s);
        s += encode->len;
    }

    *s++ = '\0';

    // return pointer to tokenized string
    return p;
}

// -----------------------------------------------------------------------
// comparing strings in c is dangerous im told

uint16_t is_token(uint32_t *table, size_t size, char *s)
{
    uint16_t lastc;
    uint32_t hash;
    lastc = strlen(s) -1;

    // trim quotes off keyword
    if((*s == '"') && (s[lastc] == '"'))
    {
        s[lastc] = '\0';
        s++;
    }

    hash = fnv_hash(s);
    return(is_keyword(table, size, hash));
}

// =======================================================================
