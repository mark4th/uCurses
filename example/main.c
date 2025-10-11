// test main
// -----------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "demo.h"

#define SLEEP      (15000000)
#define STATUS_X   (55)
#define STATUS_Y   (0)
#define WINCH_KEY  (0xec)  // randomly chosen value

uC_window_t *status_win;

extern bool winch;

// -----------------------------------------------------------------------

void hello(void);

// -----------------------------------------------------------------------
// user winch handler registered with the library (can be only one)

// we cant process a winch here because it would take way too long so
// instead we inject a key into the input stream to be handled in the main
// loop below

void my_winch(void)
{
    uC_set_key(WINCH_KEY);
}

// -----------------------------------------------------------------------

static void init_main(void)
{
    uCurses_init("json/main.json", NULL, menu_address_cb);

    status_win = uC_add_status(active_screen, STAT_SIZE,
        STATUS_X, STATUS_Y);

    // %fs set window forground to a gray scale color
    // %bs set winndow background to a gray scale color
    // %0 clear the window

    uC_win_printf(status_win, "%fs%bs%0", 9, 3);

//    uC_register_winch(my_winch);
}

// -----------------------------------------------------------------------
// called by the main loop when it receives notification

static void do_winch(void)
{
    uCurses_deInit();
    init_main();

    hello();

    winch = false;
}

// -----------------------------------------------------------------------
// lazy wait for key (spends a lot of time sleeping!)

static void wait_key(void)
{
    while (uC_test_keys() == 0)
    {
        uC_clock_sleep(SLEEP);
    }
}

// -----------------------------------------------------------------------

static void main_loop(void)
{
    size_t z1;
    size_t z2;
    size_t z3;
    uint8_t k = 0;
    char status[33];

    uC_screen_t *scr;

    do
    {
        scr = active_screen;

        z1 = uC_zone_query(uC_MEM_ZONE_DEFAULT);
        z2 = uC_zone_query(uC_MEM_ZONE_UI);
        z3 = uC_zone_query(uC_MEM_ZONE_JSON);

        sprintf(status, "%zu %zu %zu ", z1, z2, z3);
        uC_set_status(status_win, status);

        uC_scr_draw_screen(scr);

        wait_key();
        k = uC_key();

        // the user winch handler above cannot resize the user interface
        // so it injects a randomly selected non-sensical key into the
        // input stream which tells this main loop that it now needs to
        // destroy and recreate the entire user interface.

        if (k == WINCH_KEY)
        {
            do_winch();
        }

    } while (k != 0x1b);
}

// -----------------------------------------------------------------------

int main(void)
{
    uint16_t y;

    uC_screen_t *scr;
    uC_window_t *win;

    init_main();

    y = active_screen->height;

    hello();

    main_loop();

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(y - 3, 0);

    uC_scr_close(active_screen);
    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}

// =======================================================================
