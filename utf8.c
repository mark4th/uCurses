// utf8.c
// --------------------------------------------------------------------------

#define _XOPEN_SOURCE

#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <wchar.h>

#include "h/uCurses.h"

// --------------------------------------------------------------------------

utf8_encode_t *utf8_encode(uint32_t cp)
{
    wchar_t c = '\0';
    static utf8_encode_t encoded;

    *(uint32_t *)&encoded.str[0] = 0;

    if (cp < 0x80)
    {
        encoded.str[0] = cp;
        encoded.len = 1;
    }
    else if (cp < 0x0800)
    {
        encoded.str[0] = 0xc0 | (cp >> 6);
        encoded.str[1] = 0x80 | (cp & 0x3f);
        encoded.len = 2;
    }
    else if (cp < 0x10000)
    {
        encoded.str[0] = 0xe0 | (cp >> 12);
        encoded.str[1] = 0x80 | ((cp >> 6) & 0x3f);
        encoded.str[2] = 0x80 | (cp & 0x3f);
        encoded.len = 3;
    }
    else
    {
        encoded.str[0] = 0xf0 | (cp >> 18);
        encoded.str[1] = 0x80 | ((cp >> 12) & 0x3f);
        encoded.str[2] = 0x80 | ((cp >> 6) & 0x3f);
        encoded.str[3] = 0x80 | (cp & 0x3f);
        encoded.len = 4;
    }
    memcpy(&c, &encoded.str[0], encoded.len);

    // the following is tribal knowledge.  when these characters are written
    // into a window the windows curor x is incremented by 1 spot/ if nowever
    // the character we are going to write is wide we need to bump the
    // windows curosr by two slots and mark the following cell in the window
    // as being dead (0xDEADC0DE)
    encoded.width = wcwidth(c);

    return &encoded;
}

// --------------------------------------------------------------------------

void utf8_emit(uint32_t cp)
{
    uint8_t i;
    utf8_encode_t *encoded;

    if(cp != DEADCODE)
    {
        encoded = utf8_encode(cp);

        for(i = 0; i < encoded->len; i++)
        {
            // append utf8 character onto end of terminfo escape seauence
            // buffer to be blasted out to the console later
            c_emit(encoded->str[i]);
        }
    }
}

// --------------------------------------------------------------------------

uint8_t utf8_decode(uint32_t *cp, char *s)
{
    // 0xxxxxxx
    if ((uint8_t)s[0] < 0x80)
    {
        *cp = *s;
        return 1;
    }

     // 110xxxxx  10xxxxxx
    if (((uint8_t)s[0] >= 0xc0) && ((uint8_t)s[0] < 0xe0))
    {
        // TODO: if(0x80 == s[1] & 0xc0) { yay }
        *cp = (((uint8_t)s[0] & 0x1f) << 6) | ((uint8_t)s[1] & 0x3f);
        return 2;
    }

    // 1110xxxx 10xxxxxx 10xxxxxx
    if (((uint8_t)s[0] >= 0xe0) && ((uint8_t)s[0] < 0xf0))
    {
        *cp = (((uint8_t)s[0] & 0x1f) << 12) | (((uint8_t)s[1] & 0x3f) << 6) |
               ((uint8_t)s[2] & 0x3f);
        return 3;
    }

    // 11110xxx 10xxxxx 10xxxxx 10xxxxx
    if ((uint8_t)s[0] > 0xef)
    {
        *cp = (((uint8_t)s[0] & 0x7) << 18) | (((uint8_t)s[1] & 0x3f) << 12) |
              (((uint8_t)s[2] & 0x3f) << 6) | ((uint8_t)s[3] & 0x3f);
        return 4;
    }

    return -1;
}

// --------------------------------------------------------------------------
// gets number of character cells the string will use. this accounts for
// characters such as chinese characters which take up two cells worth of
// space in the console when displayed

uint16_t utf8_width(char *s)
{
    utf8_encode_t *encode;
    uint16_t width = 0;

    while(*s != '\0')
    {
        encode = utf8_encode(*s);
        s += encode->len;
        width += (encode->width != 1) ? 2 : 1;
    }
    return width;
}

// --------------------------------------------------------------------------
// calcuate the string length of a utf8 string.  characters in utf8 strings
// can be 1, 2, 3 or 4 bytes long

uint16_t utf8_strlen(char *s)
{
    utf8_encode_t *encode;
    uint16_t len = 0;

    while(*s != '\0')
    {
        encode = utf8_encode(*s);
        s += encode->len;
        len++;
    }
    return len;
}

// ==========================================================================
