// hello.c
// -----------------------------------------------------------------------

#include <stddef.h>
#include <stdio.h>

#include "demo.h"

// -----------------------------------------------------------------------

#define DARK  (uC_GRAY_01)
#define LIGHT (uC_COLOR_BROWN)

extern uC_screen_t *active_screen;
extern uC_window_t *status_win;

// -----------------------------------------------------------------------

void banner(uC_screen_t *scr, uC_window_t *win)
{
    int16_t xco;
    int16_t yco;

    xco = (scr->width / 2) - (58 / 2) - 1;
    yco = 5;

    uC_win_printf(win, "%@%fc██%fs▖   %fc██%fs▖ %fc██████%fs▖%fc██%fs▖   %fc██"
        "%fs▖%fc██████%fs▖ %fc███████%fs▖%fc███████%fs▖%fc███████%fs▖",
        xco, yco++, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▛▀▀▀▀▘%fc██%fs▌   %fc██"
        "%fs▌%fc██%fs▛▀▀%fc██%fs▖%fc██%fs▛▀▀▀▀▘%fc██%fs▛▀▀▀▀▘%fc██%fs▛▀▀▀▀▘",
        xco, yco++, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▌     %fc██%fs▌   %fc██"
        "%fs▌%fc██████%fs▛▘%fc███████%fs▖%fc█████%fs▖  %fc███████%fs▖",
        xco, yco++, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▌     %fc██%fs▌   %fc██"
        "%fs▌%fc██%fs▛▀▀%fc██%fs▌▝▀▀▀▀%fc██%fs▌%fc██%fs▛▀▀▘  ▝▀▀▀▀%fc██%fs▌",
        xco, yco++, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fs▝%fc██████%fs▛▘▝%fc██████%fs▖▝%fc██████%fs▛▘%fc██"
        "%fs▌  %fc██%fs▌%fc███████%fs▌%fc███████%fs▖%fc███████%fs▌",
        xco, yco++, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK,
        LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@ %fs▝▀▀▀▀▀▘  ▝▀▀▀▀▀▘ ▝▀▀▀▀▀▘ ▝▀▘  ▝▀▘▝▀▀▀▀▀▀▘▝▀▀▀▀▀▀▘"
        "▝▀▀▀▀▀▀▘",
        xco, yco++, DARK);

    yco++;

    xco = (scr->width / 2) - (46 / 2) - 2;
    uC_win_printf(win, "%@%fcDemo Application.  Press %U+%B+F10%B-%U- to pull down Menu",
        xco, yco++, uC_COLOR_CYAN);

    yco++;

    xco = (scr->width / 2) - (50 / 2) - 2;
    uC_win_printf(win, "%@%fcEscape quits each demo:  Escape here quits program!",
        xco, yco++, uC_COLOR_LT_GREEN);
}

// -----------------------------------------------------------------------

void hello(void)
{
    uC_screen_t *scr;
    uC_window_t *win;
    uC_list_node_t *n;

    scr = active_screen;

    n = scr->windows.head;
    win = n->payload;

    uC_clear();
    banner(scr, win);
}

// =======================================================================
