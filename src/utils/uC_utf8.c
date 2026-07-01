// uC_utf8.c
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE

#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "uCurses.h"
#include "uC_utf8.h"
#include "uC_terminfo.h"

// -----------------------------------------------------------------------
// encode a utf code point into utf-8 characters for printing

utf8_encode_t *utf8_encode(int32_t cp)
{
    static utf8_encode_t encoded;

    encoded.zero = 0;

    if (cp < 0x80)
    {
        encoded.str[0] = cp;
        encoded.len = 1;
    }
    else if (cp < 0x0800)
    {
        encoded.str[0] = 0xc0 | ((cp >> 6) & 0x1f);
        encoded.str[1] = 0x80 | (cp & 0x3f);
        encoded.len = 2;
    }
    else if (cp < 0x10000)
    {
        encoded.str[0] = 0xe0 | ((cp >> 12) & 0x0f);
        encoded.str[1] = 0x80 | ((cp >> 6) & 0x3f);
        encoded.str[2] = 0x80 | (cp & 0x3f);
        encoded.len = 3;
    }
    else if (cp < 0x110000)
    {
        encoded.str[0] = 0xf0 | ((cp >> 18) & 0x07);
        encoded.str[1] = 0x80 | ((cp >> 12) & 0x3f);
        encoded.str[2] = 0x80 | ((cp >> 6) & 0x3f);
        encoded.str[3] = 0x80 | (cp & 0x3f);
        encoded.len = 4;
    }
    // the following is tribal knowledge.  when these characters are
    // written into a window the windows curor x is incremented by 1 spot.
    // if however the character we are going to write is wide we need to
    // bump the windows cursor by two slots and mark the following cell in
    // the window as being dead (0xDEADC0DE)

    encoded.width = wcwidth((wchar_t)cp);

    return &encoded;
}

// -----------------------------------------------------------------------
// utf8 chars can be 1, 2, 3, 4, 5, ??? columns wide on the console

API int16_t uC_utf8_is_wide(uint32_t code)
{
    int32_t width = wcwidth(code);
    return ((width > 1) ? 1 : 0);
}

// -----------------------------------------------------------------------
// convert code point to string of characters and write them

API void uC_utf8_emit(uint32_t cp)
{
    uint8_t i;
    utf8_encode_t *encoded;

    // double wide characters such as chinese use up two cells in the
    // console display so must also take up two cells in the window
    // the second cell is marked as dead - this is not an error

    if (cp != (uint32_t)DEADC0DE)
    {
        encoded = utf8_encode(cp);

        for (i = 0; i < encoded->len; i++)
        {
            // append utf8 character onto end of terminfo escape seauence
            // buffer to be blasted out to the console later

            c_emit(encoded->str[i]);
        }
    }
}

// -----------------------------------------------------------------------
// extract utf8 codepoint from string of chars and return cp length

API uint8_t utf8_decode(uint32_t *cp, uint8_t *s)
{
    // 0xxxxxxx
    if ((uint8_t)s[0] < 0x80)
    {
        *cp = *s;
        return 1;
    }

    // 110xxxxx  10xxxxxx
    if ((s[0] & 0xe0) == 0xc0)
    {
        *cp = ((s[1]  & 0xc0) == 0x80)
            ? (((s[0] & 0x1f) << 6) | (s[1] & 0x3f))
            : 0xefbfbd;
        return 2;
    }

    // 1110xxxx 10xxxxxx 10xxxxxx
    if ((s[0] & 0xf0) == 0xe0)
    {
        *cp = (((s[1] & 0xc0) == 0x80) &&
              ((s[2]  & 0xc0) == 0x80))

            ? (((s[0] & 0x0f) << 12) |
               ((s[1] & 0x3f) << 6)  |
               (s[2]  & 0x3f))
            : 0xefbfbd;
        return 3;
    }

    // 11110xxx 10xxxxx 10xxxxx 10xxxxx
    if ((s[0] & 0xf8) == 0xf0)
    {
        *cp = (((s[1] & 0xc0) == 0x80) &&
               ((s[2] & 0xc0) == 0x80) &&
               ((s[3] & 0xc0) == 0x80))
            ? (((s[0] & 0x7)  << 18) |
               ((s[1] & 0x3f) << 12) |
               ((s[2] & 0x3f) << 6)  |
                (s[3] & 0x3f))
            : 0xefbfbd;
        return 4;
    }

    return -1;
}

// -----------------------------------------------------------------------
// return number of bytes for a utf8 char pointed to by *s

API uint8_t uC_utf8_char_length(uint8_t *s)
{
    uint8_t c = *(uint8_t *)s;

    if ((c < 0x80))                { return 1; }
    if ((c & 0xe0) == 0xc0)        { return 2; }
    if ((c & 0xf0) == 0xe0)        { return 3; }
    if (((c & 0xf8) == 0xf0) &&
        (c <= 0xf4))               { return 4; }

    return -1;
}

// -----------------------------------------------------------------------
// gets number of console character cells the string will use. this
// accounts for characters such as chinese which take up two cells worth
// of space in the console when displayed.

API int16_t uC_utf8_width(uint8_t *s)
{
    uint32_t cp;
    uint8_t n;
    int16_t width = 0;

    while (*s != '\0')
    {
        n = utf8_decode(&cp, s);
        s += n;
        width += wcwidth((wchar_t)cp);
    }

    return width;
}

// -----------------------------------------------------------------------
// calcuate the string length of a utf8 string.  characters in utf8
// strings can be 1, 2, 3 or 4 bytes long (this is character count not
// display coverage)

API int16_t uC_utf8_strlen(uint8_t *s)
{
    int16_t len = 0;
    uint8_t n;

    while (*s != '\0')
    {
        n = uC_utf8_char_length(s);
        s += n;
        len++;
    }

    return len;
}

// -----------------------------------------------------------------------

API int16_t uC_utf8_strncmp(uint8_t *s1, uint8_t *s2, int16_t len)
{
    uint32_t cp1, cp2;
    uint8_t n1, n2;

    while ((*s1 != '\0') && (len != 0))
    {
        n1 = utf8_decode(&cp1, s1);
        n2 = utf8_decode(&cp2, s2);

        if (cp1 != cp2)
            return (int16_t)((int32_t)cp1 - (int32_t)cp2);

        s1 += n1;
        s2 += n2;
        len--;
    }

    return 0;
}

// =======================================================================
