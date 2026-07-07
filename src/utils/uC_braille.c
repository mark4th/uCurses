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

static const uint8_t skew[] =
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

static uint8_t braille_8_pack_cell(uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4, uint8_t bit)
{
    // the high nibble of the return data will be the current bit from
    // each of d1 d2 d3 d4 and the low nibble will be the next lower order
    // bit from them.

    return (uint8_t)(
        (((d1 >> bit) & 1) << 0) |
        (((d2 >> bit) & 1) << 1) |
        (((d3 >> bit) & 1) << 2) |
        (((d4 >> bit) & 1) << 3) |
        (((d1 >> (bit - 1)) & 1) << 4) |
        (((d2 >> (bit - 1)) & 1) << 5) |
        (((d3 >> (bit - 1)) & 1) << 6) |
        (((d4 >> (bit - 1)) & 1) << 7));
}

// -----------------------------------------------------------------------

static void braille_8_emit_cells(uC_window_t *win, uint16_t *braille_data,
    uint16_t wx, uint16_t wy, uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4)
{
    uint8_t z;
    uint8_t d0;
    uint8_t bit = 7;

    for (z = 0; z < 4; z++)
    {
        d0 = braille_8_pack_cell(d1, d2, d3, d4, bit);

        if ((wx < win->width) && (wy < win->height))
        {
            braille_data[(wy * win->width) + wx] = uC_braille_xlat(d0);
        }
        wx++;
        bit -= 2;
    }
}

// -----------------------------------------------------------------------

API void uC_braille_8(uC_window_t *win, uint16_t *braille_data,
    uint8_t *map, uint16_t width)
{
    uint16_t x, y;
    uint8_t d1, d2, d3, d4;
    uint8_t *p0;

    p0 = map;

    for (y = 0; y < win->height; y++)
    {
        for (x = 0; x < width; x++)
        {
            d1 = p0[width * 0];
            d2 = p0[width * 1];
            d3 = p0[width * 2];
            d4 = p0[width * 3];

            braille_8_emit_cells(win, braille_data, x * 4, y, d1, d2, d3,
                d4);
            p0++;
        }
        p0 += (width * 3);
    }
}

// -----------------------------------------------------------------------
// braille encode map that contains 1 bits per byte of data

static uint8_t braille_1_pack_cell(const uint8_t *p0, const uint8_t *p1,
    const uint8_t *p2, const uint8_t *p3, uint16_t x)
{
    uint16_t x1 = x + 1;

    return (uint8_t)(
        ((p0[x] & 1) << 0) |
        ((p1[x] & 1) << 1) |
        ((p2[x] & 1) << 2) |
        ((p3[x] & 1) << 3) |
        ((p0[x1] & 1) << 4) |
        ((p1[x1] & 1) << 5) |
        ((p2[x1] & 1) << 6) |
        ((p3[x1] & 1) << 7));
}

// -----------------------------------------------------------------------

static void braille_1_emit_cell(uC_window_t *win, uint16_t *braille_data,
    uint16_t x, uint16_t y, uint8_t d0)
{
    if ((x < win->width) && (y < win->height))
    {
        braille_data[(y * win->width) + x] = uC_braille_xlat(d0);
    }
}

// -----------------------------------------------------------------------

API void uC_braille_1(uC_window_t *win, uint16_t *braille_data,
    uint8_t *map, uint16_t width, uint16_t height)
{
    uint16_t x, y;
    uint8_t *p0, *p1, *p2, *p3;
    uint8_t d0;

    for (y = 0; y < height / 4; y++)
    {
        p0 = map + (y * 4 * width);
        p1 = p0 + width;
        p2 = p1 + width;
        p3 = p2 + width;

        for (x = 0; x < width / 2; x++)
        {
            d0 = braille_1_pack_cell(p0, p1, p2, p3, x * 2);
            braille_1_emit_cell(win, braille_data, x, y, d0);
        }
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
