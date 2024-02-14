// test main
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "uCurses.h"
#include "uC_json.h"
#include "uC_win_printf.h"
#include "uC_keys.h"
#include "uC_terminfo.h"
#include "uC_utils.h"
#include "uC_json.h"

#include "demo.h"

#define SLEEP 15000000

extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------

#define DARK  1
#define LIGHT BROWN

#include <stdio.h>

void main_screen(void)
{
    uC_screen_t *scr;
    uC_window_t *win;
    uC_list_node_t *n;

    int16_t xco;

    uCurses_init();

    uC_json_create_ui("base.json", menu_address_cb);
    uC_alloc_status();
    uC_bar_clr_status();
    uC_menu_init();

    scr = active_screen;
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

// -----------------------------------------------------------------------

int main(void)
{
    uC_screen_t *scr;
    uC_window_t *win;

    main_screen();

    do
    {
        // we need to do this inside this loop because when we run one of
        // the demos it will kill our screen and all our windows but will
        // reactivate them for us when it returns (the call to key is what
        // actually calls the selected demo).

        scr = active_screen;
        uC_scr_draw_screen(scr);

        while(uC_test_keys() == 0)
        {
            uC_clock_sleep(SLEEP);
        }
    } while(uC_key() != 0x1b);

    uC_scr_close(active_screen);

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(10, 0);

    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}

// =======================================================================
