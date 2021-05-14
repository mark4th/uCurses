// test main
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// not an example of how to do thins properly, just test/debug code
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "../h/uCurses.h"

#include "demo.h"

#define SLEEP 15000000

// -----------------------------------------------------------------------

#define DARK  1
#define LIGHT BROWN

void main_screen(void)
{
    screen_t *scr;
    window_t *win;
    node_t *n;

    int16_t xco;

    json_create_ui("base.json", menu_address_cb);

    scr = active_screen;
    xco = (scr->width / 2) - (58 / 2) - 1;
    n = scr->windows.head;
    win = n->payload;

    win_printf(win, "%@%fc██%fs▖   %fc██%fs▖ %fc██████%fs▖%fc██%fs▖   %fc██"
        "%fs▖%fc██████%fs▖ %fc███████%fs▖%fc███████%fs▖%fc███████%fs▖",
        xco, 5, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▛▀▀▀▀▘%fc██%fs▌   %fc██"
        "%fs▌%fc██%fs▛▀▀%fc██%fs▖%fc██%fs▛▀▀▀▀▘%fc██%fs▛▀▀▀▀▘%fc██%fs▛▀▀▀▀▘",
        xco, 6, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▌     %fc██%fs▌   %fc██"
        "%fs▌%fc██████%fs▛▘%fc███████%fs▖%fc█████%fs▖  %fc███████%fs▖",
        xco, 7, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    win_printf(win, "%@%fc██%fs▌   %fc██%fs▌%fc██%fs▌     %fc██%fs▌   %fc██"
        "%fs▌%fc██%fs▛▀▀%fc██%fs▌▝▀▀▀▀%fc██%fs▌%fc██%fs▛▀▀▘  ▝▀▀▀▀%fc██%fs▌",
        xco, 8, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
        DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    win_printf(win, "%@%fs▝%fc██████%fs▛▘▝%fc██████%fs▖▝%fc██████%fs▛▘%fc██"
        "%fs▌  %fc██%fs▌%fc███████%fs▌%fc███████%fs▖%fc███████%fs▌",
        xco, 9, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK,
        LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK);

    win_printf(win, "%@ %fs▝▀▀▀▀▀▘  ▝▀▀▀▀▀▘ ▝▀▀▀▀▀▘ ▝▀▘  ▝▀▘▝▀▀▀▀▀▀▘▝▀▀▀▀▀▀▘"
        "▝▀▀▀▀▀▀▘",
        xco, 10, DARK);

    xco = (scr->width / 2) - (46 / 2) - 2;
    win_printf(win, "%@%fcDemo Application.  Press F10 to pull down Menu",
        xco, 12, CYAN);
    xco = (scr->width / 2) - (50 / 2) - 2;
    win_printf(win, "%@%fcEscape quits each demo: Escape here quits program!",
        xco, 14, LT_GREEN);
}

// -----------------------------------------------------------------------

int main(void)
{
    screen_t *scr;
    window_t *win;
    node_t *n;

    alloc_status();

    uCurses_init();

    // initialize menu key handling
    menu_init();


    main_screen();


    do
    {
        // we need to do this inside this loop because when we run one of
        // the demos it will kill our screen and all our windows but will
        // reactivate them for us when it returns (the call to key is what
        // actually calls the selected demo).

        scr = active_screen;
        n = scr->windows.head;
        win = n->payload;

        scr_draw_screen(scr);

        while(test_keys() == 0)
        {
            clock_sleep(SLEEP);
        }
    } while(key() != 0x1b);

    scr_close(active_screen);

    set_fg(WHITE);
    set_bg(BLACK);

    clear();
    cup(10, 0);
    restore_term();

    uCurses_deInit();

    printf("bye!\n");
    return 0;
}

// =======================================================================
