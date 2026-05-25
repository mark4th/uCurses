// borders.c  window border types demonstration
// -----------------------------------------------------------------------
// four windows showing: no border, single, double (named), curved (named)

#include <inttypes.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_window.h"
#include "uC_borders.h"
#include "uC_attribs.h"
#include "uC_win_printf.h"
#include "uC_switch.h"
#include "uC_keys.h"
#include "uC_terminfo.h"

// -----------------------------------------------------------------------

#define WIN_W  28
#define WIN_H   7

#define COL_1   3
#define COL_2  45
#define ROW_1   3
#define ROW_2  13

// -----------------------------------------------------------------------

static uC_attribs_t bdr_attrs =
{
    .flags.bits = ATTR_FLAG_GRAY_BG,
    .fg         = uC_COLOR_CYAN,
    .bg_gray    = uC_GRAY_04,
};

static uC_attribs_t bdr_focus_attrs =
{
    .flags.bits = (ATTR_FLAG_BOLD | ATTR_FLAG_GRAY_BG),
    .fg         = uC_COLOR_LT_GREEN,
    .bg_gray    = uC_GRAY_06,
};

// -----------------------------------------------------------------------

static uC_screen_t *scr;

// -----------------------------------------------------------------------

static uC_window_t *make_window(int16_t x, int16_t y)
{
    uC_window_t *win = uC_win_open(WIN_W, WIN_H);

    uC_scr_win_attach(scr, win);
    uC_win_set_pos(win, x, y);

    return win;
}

// -----------------------------------------------------------------------

static void build_demo(void)
{
    uC_window_t *win;

    // no border (default)
    win = make_window(COL_1, ROW_1);
    uC_win_printf(win, "%@%fc%B+No Border%B-",     1, 1, uC_COLOR_LT_WHITE);
    uC_win_printf(win, "%@%fc(uC_BDR_NONE)",       1, 3, uC_COLOR_GRAY);

    // single border, no name
    win = make_window(COL_2, ROW_1);
    uC_win_set_border(win, uC_BDR_SINGLE, bdr_attrs, bdr_focus_attrs);
    uC_win_printf(win, "%@%fc%B+Single Border%B-", 1, 1, uC_COLOR_LT_WHITE);
    uC_win_printf(win, "%@%fc(uC_BDR_SINGLE)",     1, 3, uC_COLOR_CYAN);

    // double border with name
    win = make_window(COL_1, ROW_2);
    uC_win_set_border(win, uC_BDR_DOUBLE, bdr_attrs, bdr_focus_attrs);
    uC_win_set_name(win, "double border");
    uC_win_printf(win, "%@%fc%B+Double Border%B-", 1, 1, uC_COLOR_LT_WHITE);
    uC_win_printf(win, "%@%fc(uC_BDR_DOUBLE)",     1, 3, uC_COLOR_CYAN);

    // curved border with name
    win = make_window(COL_2, ROW_2);
    uC_win_set_border(win, uC_BDR_CURVED, bdr_attrs, bdr_focus_attrs);
    uC_win_set_name(win, "curved border");
    uC_win_printf(win, "%@%fc%B+Curved Border%B-", 1, 1, uC_COLOR_LT_WHITE);
    uC_win_printf(win, "%@%fc(uC_BDR_CURVED)",     1, 3, uC_COLOR_CYAN);
}

// -----------------------------------------------------------------------

static void exit_prog(void)
{
    uC_set_key(0x1b);
}

// -----------------------------------------------------------------------

static uC_switch_t menu_vectors[] =
{
    { 0x8d9c616c, exit_prog }
};

#define VCOUNT sizeof(menu_vectors) / sizeof(menu_vectors[0])

// -----------------------------------------------------------------------

opt_t menu_address_cb(int32_t hash)
{
    size_t i;
    uC_switch_t *s = menu_vectors;

    for (i = 0; i < VCOUNT; i++)
    {
        if (hash == s->option)
        {
            return s->vector;
        }
        s++;
    }

    return NULL;
}

// -----------------------------------------------------------------------

int main(void)
{
    uint8_t k;

    scr = uCurses_init("json/borders.json", NULL, menu_address_cb);

    build_demo();
    uC_scr_draw_screen(scr);

    do
    {
        while (uC_test_keys() == 0)
            ;
        k = uC_key();
    } while (k != 0x1b);

    uC_console_reset_attrs();
    uC_clear();
    uC_cup(10, 0);

    uC_scr_close(scr);
    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}

// =======================================================================
