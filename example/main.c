// test main
// -----------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "demo.h"

#define SLEEP 15000000

uC_window_t *status_win;

char status[33];

// -----------------------------------------------------------------------

void hello(void);

// -----------------------------------------------------------------------

int main(void)
{
    uC_screen_t *scr;
    uC_window_t *win;

    hello();

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
