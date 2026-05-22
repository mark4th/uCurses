// console.c -- uCurses console API without a TUI
// -----------------------------------------------------------------------
// demonstrates cursor positioning, indexed colors, gray scale, and 24-bit
// RGB using only the console-level API.  no windows, no screens.

#include <stdint.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_attribs.h"

// -----------------------------------------------------------------------

#define ROW_COLORS  (2)
#define ROW_GRAY    (4)
#define ROW_RGB     (6)
#define LEFT_MARGIN (4)

// -----------------------------------------------------------------------
// set background to an RGB color and print a two-cell block

static void swatch(uint8_t r, uint8_t g, uint8_t b)
{
    uC_console_set_rgb_bg(r, g, b);
    uC_terminfo_flush();
    printf("  ");
}

// -----------------------------------------------------------------------

int main(void)
{
    uint8_t i;

    // init without JSON, menus, or a screen — console API only
    uCurses_init(NULL, NULL, NULL);
    uC_clear();
    uC_curoff();

    // row 1: all 16 indexed terminal colors
    uC_cup(LEFT_MARGIN, ROW_COLORS);
    uC_terminfo_flush();
    for (i = 0; i < 16; i++)
    {
        uC_console_set_bg(i);
        uC_terminfo_flush();
        printf("  ");
    }

    // row 2: 24-step terminal gray scale
    uC_cup(LEFT_MARGIN, ROW_GRAY);
    uC_terminfo_flush();
    for (i = 0; i < 24; i++)
    {
        uC_console_set_gray_bg(i);
        uC_terminfo_flush();
        printf("  ");
    }

    // row 3: 48-cell 24-bit RGB gradient (red → green → blue)
    uC_cup(LEFT_MARGIN, ROW_RGB);
    for (i = 0; i < 48; i++)
    {
        uint8_t r = (i < 24) ? (255 - i * 10) : 0;
        uint8_t g = (i < 24) ? (i * 10)       : (255 - (i - 24) * 10);
        uint8_t b = (i < 24) ? 0              : ((i - 24) * 10);
        swatch(r, g, b);
    }

    uC_console_reset_attrs();
    uC_cup(LEFT_MARGIN, ROW_RGB + 3);
    uC_terminfo_flush();
    uC_curon();

    uCurses_deInit();

    printf("\n");
    return 0;
}

// =======================================================================
