// scroll.c  -  window scroll / pan / copy demonstration
// -----------------------------------------------------------------------
// A backing window holds pure grid content and is scrolled/panned by the
// uC_win_scroll_*() functions.  After each operation uC_win_copy_win()
// copies the backing buffer to the visible window and row labels are
// stamped as a fixed overlay at columns 0-3, allowing unlimited pan in
// all directions.
//
// Arrow keys:  pan    ESC: quit

#include <inttypes.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_window.h"
#include "uC_attribs.h"
#include "uC_borders.h"
#include "uC_win_printf.h"
#include "uC_keys.h"
#include "uC_mouse.h"
#include "uC_status.h"
#include "uC_terminfo.h"
#include "uC_widgets.h"

// -----------------------------------------------------------------------

#define WIN_W    80
#define WIN_H    22
#define WIN_X     5
#define WIN_Y     5

// -----------------------------------------------------------------------

static uC_attribs_t bdr_attrs =
{
    .flags.bits = ATTR_FLAG_GRAY_BG,
    .fg         = uC_COLOR_CYAN,
    .bg_gray    = uC_GRAY_04,
};

static uC_attribs_t bdr_focus_attrs =
{
    .flags.bits = ATTR_FLAG_GRAY_BG,
    .fg         = uC_COLOR_LT_CYAN,
    .bg_gray    = uC_GRAY_06,
};

// -----------------------------------------------------------------------

static uC_screen_t *scr;
static uC_window_t *win;
static uC_window_t *grid_win;   // backing buffer: pure grid, no labels
static uC_window_t *stat_win;

// virtual origin: which grid cell is at window position (0,0)
static int16_t org_x = 0;
static int16_t org_y = 0;

// -----------------------------------------------------------------------
// cursor key handlers

static void key_up(void)    { uC_set_key(WIDGET_KEY_UP);    }
static void key_down(void)  { uC_set_key(WIDGET_KEY_DOWN);  }
static void key_left(void)  { uC_set_key(WIDGET_KEY_LEFT);  }
static void key_right(void) { uC_set_key(WIDGET_KEY_RIGHT); }

// -----------------------------------------------------------------------
// draw one grid cell (no labels) at the current cursor position of grid_win.
// +| markers appear every 10 columns starting at vcol 4.

static void draw_grid_cell(int16_t vcol)
{
    if (!((vcol - 4) % 10))
    {
        uC_win_printf(grid_win, "%fc%B+|%B-", uC_COLOR_LT_WHITE);
    }
    else
    {
        uC_win_printf(grid_win, "%fs.", uC_GRAY_05);
    }
}

// -----------------------------------------------------------------------
// stamp row labels as a fixed overlay at window columns 0-3

static void draw_labels(void)
{
    int16_t r;
    int16_t vrow;
    char label[5];

    for (r = 0; r < WIN_H; r++)
    {
        vrow = org_y + r;
        snprintf(label, sizeof(label), "%02d: ", (vrow % 100 + 100) % 100);
        uC_win_printf(win, "%@", 0, r);

        if (vrow & 1)
        {
            uC_win_printf(win, "%fc%s", uC_COLOR_LT_GREEN, label);
        }
        else
        {
            uC_win_printf(win, "%fs%s", uC_GRAY_07, label);
        }
    }
}

// -----------------------------------------------------------------------
// copy grid backing buffer to visible window then stamp labels

static void sync_win(void)
{
    uC_win_copy_win(win, grid_win);
    draw_labels();
}

// -----------------------------------------------------------------------
// draw one grid row into grid_win

static void draw_grid_row(int16_t win_r, int16_t vrow)
{
    int16_t c;

    (void)vrow;
    uC_win_printf(grid_win, "%@", 0, win_r);

    for (c = 0; c < WIN_W; c++)
    {
        draw_grid_cell(org_x + c);
    }
}

// -----------------------------------------------------------------------
// draw one grid column into grid_win

static void draw_grid_col(int16_t win_c, int16_t vcol)
{
    int16_t r;

    for (r = 0; r < WIN_H; r++)
    {
        uC_win_printf(grid_win, "%@", win_c, r);
        draw_grid_cell(vcol);
    }
}

// -----------------------------------------------------------------------
// initial fill: draw every row into grid_win then sync to visible window

static void fill_content(void)
{
    int16_t r;

    grid_win->flags |= uC_WIN_LOCKED;
    win->flags      |= uC_WIN_LOCKED;

    for (r = 0; r < WIN_H; r++)
    {
        draw_grid_row(r, org_y + r);
    }

    sync_win();
}

// -----------------------------------------------------------------------

static void scroll_up(void)
{
    uC_win_scroll_up(grid_win);
    org_y++;
    draw_grid_row(WIN_H - 1, org_y + WIN_H - 1);
    sync_win();
}

static void scroll_dn(void)
{
    uC_win_scroll_dn(grid_win);
    org_y--;
    draw_grid_row(0, org_y);
    sync_win();
}

static void scroll_lt(void)
{
    uC_win_scroll_lt(grid_win);
    org_x++;
    draw_grid_col(WIN_W - 1, org_x + WIN_W - 1);
    sync_win();
}

static void scroll_rt(void)
{
    uC_win_scroll_rt(grid_win);
    org_x--;
    draw_grid_col(0, org_x);
    sync_win();
}

// -----------------------------------------------------------------------

static void handle_mouse(void)
{
    switch (uC_mouse_event.button)
    {
        case UC_MOUSE_WHEEL_UP: scroll_dn(); break;
        case UC_MOUSE_WHEEL_DN: scroll_up(); break;
        case UC_MOUSE_WHEEL_LT: scroll_rt(); break;
        case UC_MOUSE_WHEEL_RT: scroll_lt(); break;
    }
}

// -----------------------------------------------------------------------

int main(void)
{
    uint8_t k;
    uC_kh_t saved_kh;
    uC_window_t *backdrop;

    scr = uCurses_init(NULL, NULL, NULL);

    saved_kh = uC_alloc_kh();
    uC_set_key_action(K_CUU1, key_up);
    uC_set_key_action(K_CUD1, key_down);
    uC_set_key_action(K_CUB1, key_left);
    uC_set_key_action(K_CUF1, key_right);

    backdrop = scr->backdrop;
    uC_win_printf(backdrop, "%fs%bs%0", uC_GRAY_03, uC_GRAY_02);
    uC_win_printf(backdrop, "%@%fs%s", WIN_X, 1, uC_GRAY_07,
        "uC_win_scroll_up/dn/lt/rt_n() — also available as win_printf %up(n) %dn(n) %lt(n) %rt(n)");

    win = uC_win_open(WIN_W, WIN_H);
    uC_scr_win_attach(scr, win);
    uC_win_set_pos(win, WIN_X, WIN_Y);
    uC_win_set_border(win, uC_BDR_SINGLE, bdr_attrs, bdr_focus_attrs);
    uC_win_set_name(win, "scroll demo");

    grid_win = uC_win_open(WIN_W, WIN_H);
    // not attached to screen - backing buffer only

    stat_win = uC_add_status(scr, WIN_W, WIN_X, WIN_Y + WIN_H + 1);
    uC_win_printf(stat_win, "%fs%bs%0", uC_GRAY_09, uC_GRAY_03);
    uC_win_printf(stat_win, "%0%fs Arrows: pan   ESC: quit", uC_GRAY_09);

    fill_content();

    uC_mouse_enable();
    uC_scr_draw_screen(scr);

    do
    {
        while (!uC_test_keys())
            ;
        k = uC_key();

        switch (k)
        {
            case WIDGET_KEY_UP:    scroll_dn();    break;
            case WIDGET_KEY_DOWN:  scroll_up();    break;
            case WIDGET_KEY_LEFT:  scroll_rt();    break;
            case WIDGET_KEY_RIGHT: scroll_lt();    break;
            case WIDGET_KEY_MOUSE: handle_mouse(); break;
        }

        uC_scr_draw_screen(scr);

    } while (k != 0x1b);

    uC_release_kh(saved_kh);
    uC_mouse_disable();
    uC_console_reset_attrs();
    uC_clear();
    uC_cup(10, 0);

    uC_scr_close(scr);
    uCurses_deInit();

    printf("Au revoir!\n");

    return 0;
}

// =======================================================================
