// utf8.c
// --------------------------------------------------------------------------

#include <inttypes.h>
#include <unistd.h>

// --------------------------------------------------------------------------

void utf8_emit(uint32_t cp)
{
    uint8_t len;
    uint8_t str[4];

    if (cp < 0x80)
    {
        str[0] = cp;
        len = 1;
    }
    else if (cp < 0x0800)
    {
        str[0] = 0xc0 | (cp >> 6);
        str[1] = 0x80 | (cp & 0x3f);
        len = 2;
    }
    else if (cp < 0x10000)
    {
        str[0] = 0xe0 | (cp >> 12);
        str[1] = 0x80 | ((cp >> 6) & 0x3f);
        str[2] = 0x80 | (cp & 0x3f);
        len = 3;
    }
    else
    {
        str[0] = 0xf0 | (cp >> 18);
        str[1] = 0x80 | ((cp >> 12) & 0x3f);
        str[2] = 0x80 | ((cp >> 6) & 0x3f);
        str[3] = 0x80 | (cp & 0x3f);
        len = 4;
    }
    write(1, &str[0], len);
}

// --------------------------------------------------------------------------

uint8_t utf8_decode(uint32_t *cp, uint8_t *s)
{
    if (s[0] < 0x80)
    {
        *cp = *s;
        return 1;
    }

    if ((s[0] >= 0xb0) && (s[0] < 0xe0))
    {
        *cp = ((s[0] & 0x1f) << 6) | (s[1] & 0x3f);
        return 2;
    }

    if ((s[0] >= 0xe0) && (s[0] < 0xf0))
    {
        *cp = ((s[0] & 0xf) << 12) | ((s[1] & 0x3f) << 6) |
               (s[2] & 0x3f);
        return 3;
    }

    if (!(s[0] & 0x8))
    {
        *cp = ((s[0] & 0x7) << 18) | ((s[1] & 0x3f) << 12) |
              ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
        return 4;
    }

    return -1;
}

#include <stdio.h>
void test(void)
{
    uint32_t cp;
    uint8_t utf8_str[] = "俪俨俩俪俭修俯";
    uint8_t *p = &utf8_str[0];
    uint8_t n;

    while(0 != *p)
    {
       n = utf8_decode(&cp, p);
       printf(" %x ", cp);
       utf8_emit(cp);
       p += n;
    }
    printf("\n");
}

// ==========================================================================
