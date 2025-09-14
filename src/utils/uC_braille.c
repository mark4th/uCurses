// braille.c   the braille engine - graphics modes within a text mode !!!
// -----------------------------------------------------------------------

#include <inttypes.h>

#include "uCurses.h"
#include "uC_window.h"

// -----------------------------------------------------------------------
// every glyph here can be thougt of as an 8 bit binary value where the
// low order nibble is on the right and the high order nibble on the left.
// unfortunately the guy who layed this out was high on cocain on that day
// and jumbued up the order of the table..

// These glyphs are all shown in numerical order from 0x00 to 0xff.  the
// number to the left gives the offset to each 8 byte sequence from the
// start of the table

// 00  ⠁⠂⠃⠄⠅⠆⠇    40 ⡀⡁⡂⡃⡄⡅⡆⡇    08 ⠈⠉⠊⠋⠌⠍⠎⠏    48 ⡈⡉⡊⡋⡌⡍⡎⡏
// 10 ⠐⠑⠒⠓⠔⠕⠖⠗    50 ⡐⡑⡒⡓⡔⡕⡖⡗    18 ⠘⠙⠚⠛⠜⠝⠞⠟    58 ⡘⡙⡚⡛⡜⡝⡞⡟
// 20 ⠠⠡⠢⠣⠤⠥⠦⠧    60 ⡠⡡⡢⡣⡤⡥⡦⡧    28 ⠨⠩⠪⠫⠬⠭⠮⠯    68 ⡨⡩⡪⡫⡬⡭⡮⡯
// 30 ⠰⠱⠲⠳⠴⠵⠶⠷    70 ⡰⡱⡲⡳⡴⡵⡶⡷    38 ⠸⠹⠺⠻⠼⠽⠾⠿    78 ⡸⡹⡺⡻⡼⡽⡾⡿
// 80 ⢀⢁⢂⢃⢄⢅⢆⢇    C0 ⣀⣁⣂⣃⣄⣅⣆⣇    88 ⢈⢉⢊⢋⢌⢍⢎⢏    C8 ⣈⣉⣊⣋⣌⣍⣎⣏
// 90 ⢐⢑⢒⢓⢔⢕⢖⢗    D0 ⣐⣑⣒⣓⣔⣕⣖⣗    98 ⢘⢙⢚⢛⢜⢝⢞⢟    D8 ⣘⣙⣚⣛⣜⣝⣞⣟
// A0 ⢠⢡⢢⢣⢤⢥⢦⢧    E0 ⣠⣡⣢⣣⣤⣥⣦⣧    A8 ⢨⢩⢪⢫⢬⢭⢮⢯    E8 ⣨⣩⣪⣫⣬⣭⣮⣯
// B0 ⢰⢱⢲⢳⢴⢵⢶⢷    F0 ⣰⣱⣲⣳⣴⣵⣶⣷    B8 ⢸⢹⢺⢻⢼⢽⢾⢿    F8 ⣸⣹⣺⣻⣼⣽⣾⣿

static uint8_t skew[] =
{
    0x00, 0x40, 0x08, 0x48, 0x10, 0x50, 0x18, 0x58,
    0x20, 0x60, 0x28, 0x68, 0x30, 0x70, 0x38, 0x78,
    0x80, 0xC0, 0x88, 0xC8, 0x90, 0xD0, 0x98, 0xD8,
    0xA0, 0xE0, 0xA8, 0xE8, 0xB0, 0xF0, 0xB8, 0xF8
};

// -----------------------------------------------------------------------
// translates value 0x00 through 0xff into the assinine order above

API int16_t uC_braille_xlat(uint8_t chr)
{
    int16_t code;
    uint16_t i;

    i = (chr >> 3);         // gives index into above skew table
    code = skew[i];         // err yea index
    code += (chr & 7);      // index into the block of 8 to get char

    return (code + 0x2800); // utf8 0x2800 to 0x28ff
}

// -----------------------------------------------------------------------
// braille encode map that contains 8 bits per byte of data

API void uC_braille_8(uC_window_t *win, uint16_t *braille_data,
    uint8_t *map, uint16_t width)
{
    uint8_t x, y, z;
    uint8_t d0, d1, d2, d3, d4;
    uint8_t *p0;
    uint16_t wx = 0, wy = 0;

    p0 = map;

    for (y = 0; y < win->height; y++)
    {
        for (x = 0; x < width; x++)
        {
            d1 = p0[width * 0];
            d2 = p0[width * 1];
            d3 = p0[width * 2];
            d4 = p0[width * 3];

            for (z = 0; z < 4; z++)
            {
                // move one bit out of each of d1/d2/d3 and d4 into d0
                d0 = 0;

                d0 >>= 1;     d0 |= d1 & 0x80;     d1 <<= 1;
                d0 >>= 1;     d0 |= d2 & 0x80;     d2 <<= 1;
                d0 >>= 1;     d0 |= d3 & 0x80;     d3 <<= 1;
                d0 >>= 1;     d0 |= d4 & 0x80;     d4 <<= 1;

                // move another bit out of each of d1/d2/d3 and d4 into d0

                d0 >>= 1;     d0 |= d1 & 0x80;     d1 <<= 1;
                d0 >>= 1;     d0 |= d2 & 0x80;     d2 <<= 1;
                d0 >>= 1;     d0 |= d3 & 0x80;     d3 <<= 1;
                d0 >>= 1;     d0 |= d4 & 0x80;     d4 <<= 1;

                // thats 8 bits copied into d0

                if ((wx < win->width) && (wy < win->height))
                {
                    braille_data[(wy * win->width) + wx] =
                        uC_braille_xlat(d0);
                }
                wx++;
            }
            p0++;
        }
        p0 += (width * 3);
        wx = 0;
        wy++;
    }
}

// -----------------------------------------------------------------------
// braille encode map that contains 1 bits per byte of data

API void uC_braille_1(uC_window_t *win, uint16_t *braille_data,
    uint8_t *map, uint16_t width, uint16_t height)
{
    uint8_t x, y;
    uint8_t *p0, *p1, *p2, *p3;
    uint8_t d0;

    p0 = map;

    for (y = 0; y < height / 4; y++)
    {
        p1 = p0 + width;
        p2 = p1 + width;
        p3 = p2 + width;

        for (x = 0; x < width / 2; x++)
        {
            d0 = 0;     d0 |= (*p0++ << 7);
            d0 >>= 1;   d0 |= (*p1++ << 7);
            d0 >>= 1;   d0 |= (*p2++ << 7);
            d0 >>= 1;   d0 |= (*p3++ << 7);
            d0 >>= 1;   d0 |= (*p0++ << 7);
            d0 >>= 1;   d0 |= (*p1++ << 7);
            d0 >>= 1;   d0 |= (*p2++ << 7);
            d0 >>= 1;   d0 |= (*p3++ << 7);

            if ((x < win->width) && (y < win->height))
            {
                braille_data[(y * win->width) + x] = uC_braille_xlat(d0);
            }
        }
        p0 += (width * 3);
    }
}

// -----------------------------------------------------------------------

API void uC_draw_braille(uC_window_t *win, uint16_t *braille_data)
{
    uint16_t x;
    uint16_t y;

    for (y = 0; y < win->height; y++)
    {
        uC_win_cup(win, 0, y);

        for (x = 0; x < win->width; x++)
        {
            uC_win_emit(win, braille_data[(y * win->width) + x]);
        }
    }
}

// =======================================================================
