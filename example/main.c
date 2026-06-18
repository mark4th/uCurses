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

uC_screen_t *active_screen;
uC_window_t *status_win;
static uint32_t resize_count;

// -----------------------------------------------------------------------

void hello(void);

// -----------------------------------------------------------------------
// example fatal error handler: called by uC_abort() just before exit(1).
// the terminal may be in raw mode at this point so writing to stderr is
// unreliable; logging to a file is the safest option.  the handler may
// also longjmp() back to a recovery point to prevent the library's exit.

static void on_fatal(const char *msg)
{
    FILE *log = fopen("ucurses_error.log", "a");

    if (log)
    {
        fprintf(log, "%s\n", msg);
        fclose(log);
    }
}

// -----------------------------------------------------------------------

static void init_main(void)
{
    active_screen = uCurses_init("json/main.json", NULL, menu_address_cb);

    status_win = uC_add_status(active_screen, STAT_SIZE,
        STATUS_X, STATUS_Y);

    // %fs set window forground to a gray scale color
    // %bs set winndow background to a gray scale color
    // %0 clear the window

    uC_win_printf(status_win, "%fs%bs%0", 9, 3);

}

// -----------------------------------------------------------------------
// called by the main loop when it receives notification

static void wait_winch_quiet(void)
{
    do
    {
        uC_winch_ack();
        uC_clock_sleep(SLEEP * 4);
    } while (uC_winch_pending());
}

// -----------------------------------------------------------------------

static void do_winch(void)
{
    wait_winch_quiet();
    resize_count++;

    uCurses_deInit();
    init_main();

    if (active_screen != NULL)
    {
        hello();
        uC_clear();
        uC_scr_draw_screen(active_screen);
    }
}

// -----------------------------------------------------------------------
// lazy wait for key (spends a lot of time sleeping!)

static bool wait_key(void)
{
    while ((uC_test_keys() == 0) && !uC_winch_pending())
    {
        uC_clock_sleep(SLEEP);
    }

    return uC_winch_pending();
}

// -----------------------------------------------------------------------

static void main_loop(void)
{
    size_t z1;
    size_t z2;
    size_t z3;
    uint8_t k = 0;
    char status[STAT_SIZE];

    uC_screen_t *scr;

    do
    {
        scr = active_screen;

        z1 = uC_zone_query(uC_MEM_ZONE_DEFAULT);
        z2 = uC_zone_query(uC_MEM_ZONE_UI);
        z3 = uC_zone_query(uC_MEM_ZONE_JSON);

        snprintf(status, sizeof(status), "%zu %zu %zu R%" PRIu32 " ",
            z1, z2, z3, resize_count);
        uC_set_status(status_win, status);

        uC_scr_draw_screen(scr);

        if (wait_key())
        {
            do_winch();
            continue;
        }

        k = uC_key();

    } while (k != 0x1b);
}

// -----------------------------------------------------------------------

int main(void)
{
    uint16_t y;

    uC_screen_t *scr;
    uC_window_t *win;

    uC_set_fatal_handler(on_fatal);

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
