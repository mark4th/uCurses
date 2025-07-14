// hello.c
// -----------------------------------------------------------------------

#include <stddef.h>

#include "demo.h"

// -----------------------------------------------------------------------

#define DARK  1
#define LIGHT BROWN

extern uC_screen_t *active_screen;
extern uC_window_t *status_win;

// -----------------------------------------------------------------------

void hello(void)
{
    uC_screen_t *scr;
    uC_window_t *win;
    uC_list_node_t *n;

    int16_t xco;

    uCurses_init();

    uC_json_file_create_ui("json/main.json", menu_address_cb);
    uC_menu_init();

    scr = active_screen;

    status_win = uC_add_status(scr, 32, 55, 0);

    uC_set_status(status_win, status);
    uC_clr_status(status_win);

    xco = (scr->width / 2) - (58 / 2) - 1;
    n = scr->windows.head;
    win = n->payload;

    uC_win_printf(win, "%@%fc██%fs▖   %fc██%fs▖ %fc██████%fs▖%fc██%fs▖   %fc██"
        "%fs▖%fc██████%fs▖ %fc███████%fs▖%fc███████%fs▖%fc███████%fs▖",
        xco, 5, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▛▀▀▀▀▘%fc██%fs▌   %fc██"
        "%fs▌%fc██%fs▛▀▀%fc██%fs▖%fc██%fs▛▀▀▀▀▘%fc██%fs▛▀▀▀▀▘%fc██%fs▛▀▀▀▀▘",
        xco, 6, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▌     %fc██%fs▌   %fc██"
        "%fs▌%fc██████%fs▛▘%fc███████%fs▖%fc█████%fs▖  %fc███████%fs▖",
        xco, 7, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▌     %fc██%fs▌   %fc██"
        "%fs▌%fc██%fs▛▀▀%fc██%fs▌▝▀▀▀▀%fc██%fs▌%fc██%fs▛▀▀▘  ▝▀▀▀▀%fc██%fs▌",
        xco, 8, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@%fs▝%fc██████%fs▛▘▝%fc██████%fs▖▝%fc██████%fs▛▘%fc██"
        "%fs▌  %fc██%fs▌%fc███████%fs▌%fc███████%fs▖%fc███████%fs▌",
        xco, 9, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK,
        LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    uC_win_printf(win, "%@ %fs▝▀▀▀▀▀▘  ▝▀▀▀▀▀▘ ▝▀▀▀▀▀▘ ▝▀▘  ▝▀▘▝▀▀▀▀▀▀▘▝▀▀▀▀▀▀▘"
        "▝▀▀▀▀▀▀▘",
        xco, 10, DARK);

    xco = (scr->width / 2) - (46 / 2) - 2;
    uC_win_printf(win, "%@%fcDemo Application.  Press %U+%B+F10%B-%U- to pull down Menu",
        xco, 12, CYAN);

    xco = (scr->width / 2) - (50 / 2) - 2;
    uC_win_printf(win, "%@%fcEscape quits each demo:  Escape here quits program!",
        xco, 14, LT_GREEN);
}

// =======================================================================
