// utf8.c
// --------------------------------------------------------------------------

#include <inttypes.h>
#include <unistd.h>

// --------------------------------------------------------------------------

static char utf8[4];        // compiled char from code point
static uint8_t u8_n = 0;    // number of chars in above array

// --------------------------------------------------------------------------

void u8_emit(void)
{
    write(1, &utf8[0], u8_n);
    u8_n = 0;
}

// --------------------------------------------------------------------------
// compile next byte of utf-8 char into above buffer

void u8(uint32_t code, uint8_t shift, uint8_t mask1, uint8_t mask2)
{
    uint8_t c;

    c = (code >> shift);
    c &= mask1;
    c |= mask2;

    utf8[u8_n++] = c;
}

// --------------------------------------------------------------------------

static void utf_0x800(uint32_t code)
{
    u8(code,  6, 0x1f, 0xc0);
    u8(code,  0, 0x3f, 0x80);
    u8_emit();
}

// --------------------------------------------------------------------------

static void utf_0x10000(uint32_t code)
{
    u8(code, 12, 0x0f, 0xe0);       // code point, shift, mask1, mask2
    u8(code,  6, 0x3f, 0x80);
    u8(code,  0, 0x3f, 0x80);
    u8_emit();
}

// --------------------------------------------------------------------------

static void utf_0x110000(uint32_t code)
{
    u8(code, 18,    7, 0xf0);
    u8(code, 12, 0x3f, 0x80);
    u8(code,  6, 0x3f, 0x80);
    u8(code,  0, 0x3f, 0x80);
    u8_emit();
}

// --------------------------------------------------------------------------

void utf8_emit(uint32_t code)
{
    if(code < 0x7f)
    {
        u8(code, 0, 0x7f, 0);
        u8_emit();
    }
    else if(code < 0x800)
    {
        utf_0x800(code);
    }
    else if(code <0x100000)
    {
        utf_0x10000(code);
    }
    else if(code < 0x110000)
    {
        utf_0x110000(code);
    }
}

// ==========================================================================
