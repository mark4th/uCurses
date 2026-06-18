// hello.c
// -----------------------------------------------------------------------

#include <stddef.h>
#include <stdio.h>

#include "demo.h"

// -----------------------------------------------------------------------

#define DARK  (uC_GRAY_01)
#define LIGHT (uC_COLOR_BROWN)

#define BANNER_WIDTH  (58)
#define BANNER_HEIGHT (15)

extern uC_screen_t *active_screen;
extern uC_window_t *status_win;

// -----------------------------------------------------------------------

static int16_t center_x(int16_t width, int16_t text_width)
{
    if (width <= text_width)
    {
        return 0;
    }

    return (width - text_width) / 2;
}

// -----------------------------------------------------------------------

static void compact_hello(uC_screen_t *scr, uC_window_t *win)
{
    int16_t xco;
    int16_t yco;

    if ((scr == NULL) || (win == NULL) ||
        (win->width <= 0) || (win->height <= 0))
    {
        return;
    }

    yco = (win->height > 2) ? 1 : 0;

    xco = center_x(win->width, 12);
    uC_win_printf(win, "%@%fcuCurses demo", UC_XY(xco, yco++),
        uC_COLOR_CYAN);

    if (yco < win->height)
    {
        xco = center_x(win->width, 24);
        uC_win_printf(win, "%@%fcResize larger for banner",
            UC_XY(xco, yco++), uC_COLOR_LT_GREEN);
    }

    if (yco < win->height)
    {
        xco = center_x(win->width, 20);
        uC_win_printf(win, "%@%fcEscape quits program",
            UC_XY(xco, yco), uC_COLOR_LT_GREEN);
    }
}

// -----------------------------------------------------------------------

void banner(uC_screen_t *scr, uC_window_t *win)
{
    int16_t xco;
    int16_t yco;

    if ((scr == NULL) || (win == NULL) ||
        (win->width < BANNER_WIDTH) || (win->height < BANNER_HEIGHT))
    {
        compact_hello(scr, win);
        return;
    }

    xco = center_x(win->width, BANNER_WIDTH);
    yco = 5;

    uC_win_printf(win, "%@%fc██%fs▖   %fc██%fs▖ %fc██████%fs▖%fc██%fs▖   %fc██"
        "%fs▖%fc██████%fs▖ %fc███████%fs▖%fc███████%fs▖%fc███████%fs▖",
        UC_XY(xco, yco++), LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▛▀▀▀▀▘%fc██%fs▌   %fc██"
        "%fs▌%fc██%fs▛▀▀%fc██%fs▖%fc██%fs▛▀▀▀▀▘%fc██%fs▛▀▀▀▀▘%fc██%fs▛▀▀▀▀▘",
        UC_XY(xco, yco++), LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▌     %fc██%fs▌   %fc██"
        "%fs▌%fc██████%fs▛▘%fc███████%fs▖%fc█████%fs▖  %fc███████%fs▖",
        UC_XY(xco, yco++), LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▌     %fc██%fs▌   %fc██"
        "%fs▌%fc██%fs▛▀▀%fc██%fs▌▝▀▀▀▀%fc██%fs▌%fc██%fs▛▀▀▘  ▝▀▀▀▀%fc██%fs▌",
        UC_XY(xco, yco++), LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fs▝%fc██████%fs▛▘▝%fc██████%fs▖▝%fc██████%fs▛▘%fc██"
        "%fs▌  %fc██%fs▌%fc███████%fs▌%fc███████%fs▖%fc███████%fs▌",
        UC_XY(xco, yco++), DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK,
        LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@ %fs▝▀▀▀▀▀▘  ▝▀▀▀▀▀▘ ▝▀▀▀▀▀▘ ▝▀▘  ▝▀▘▝▀▀▀▀▀▀▘▝▀▀▀▀▀▀▘"
        "▝▀▀▀▀▀▀▘",
        UC_XY(xco, yco++), DARK);

    yco++;

    xco = center_x(win->width, 50);
    uC_win_printf(win, "%@%fcDemo Application.  Press %U+%B+F10%B-%U- to pull down Menu",
        UC_XY(xco, yco++), uC_COLOR_CYAN);

    yco++;

    xco = center_x(win->width, 54);
    uC_win_printf(win, "%@%fcEscape quits each demo:  Escape here quits program!",
        UC_XY(xco, yco++), uC_COLOR_LT_GREEN);
}

// -----------------------------------------------------------------------

void hello(void)
{
    uC_screen_t *scr;
    uC_window_t *win;
    uC_list_node_t *n;

    scr = active_screen;

    if (scr == NULL)
    {
        return;
    }

    n = scr->windows.head;

    if (n == NULL)
    {
        return;
    }

    win = n->payload;

    if (win == NULL)
    {
        return;
    }

    uC_clear();
    banner(scr, win);
}

// =======================================================================
