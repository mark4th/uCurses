// uC_json_build.c   - finish construction of json defined interface
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_window.h"
#include "json.h"

// -----------------------------------------------------------------------

#ifdef UC_JSON

// -----------------------------------------------------------------------

extern json_vars_t *json_vars;
extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------
// verify window is within bounds of screen

static void bounds_check(uC_window_t *win)
{
    int16_t xco = win->xco;
    int16_t yco = win->yco;
    int16_t z   = (win->flags & uC_WIN_BOXED) ? 1 : 0;

    if (((xco + win->width  + z) > json_vars->console_width)  ||
        ((yco + win->height + z) > json_vars->console_height) ||
        (xco < z) || (yco < z))
    {
        json_error("Window outside bounds of screen");
    }
}

// -----------------------------------------------------------------------
// fix far window position

static void fix_win(uC_screen_t *scr, uC_window_t *win)
{
    uC_window_t *bd    = scr->backdrop;
    int16_t      fudge = 1 + ((bd != NULL) && (bd->flags & uC_WIN_BOXED));

    if (win->xco == uC_WIN_FAR)
    {
        win->xco = scr->width  - (win->width  + fudge);
    }
    if (win->yco == uC_WIN_FAR)
    {
        win->yco = scr->height - (win->height + fudge);
    }
}

// -----------------------------------------------------------------------
// complete init of windows now we know width/height etc

static void fix_windows(uC_screen_t *scr)
{
    uC_window_t    *win = scr->backdrop;
    uC_list_node_t *n1;

    if (win != NULL)
    {
        win->xco    = 1;
        win->yco    = 1;
        win->width  = scr->width  - 2;
        win->height = scr->height - 2;
        win->screen = scr;
        if (win_alloc(win) != 0)
            json_error("Unable to allocate backdrop window");
        uC_win_clear(win);
    }

    n1 = uC_list_scan(&scr->windows, NULL);

    while (n1 != NULL)
    {
        win = n1->payload;
        fix_win(scr, win);
        bounds_check(win);
        if (win_alloc(win) != 0)
            json_error("Unable to allocate window");
        win->blank = 0x20;
        uC_win_clear(win);
        n1 = uC_list_scan(NULL, n1);
    }
}

// -----------------------------------------------------------------------

static void fix_menus(uC_screen_t *scr)
{
    menu_bar_t  *bar = scr->menu_bar;
    pulldown_t  *pd;
    uC_window_t *win;
    int16_t      i, j;

    win = uC_win_open(scr->width, 1);
    if (!win)
    {
        json_error("Unable to create window for menu bar");
        return; // prevent scan-build false positive
    }

    bar->window   = win;
    win->screen   = scr;
    win->flags    = uC_WIN_LOCKED;
    scr->menu_bar = bar;
    bar->xco      = 2;
    win->blank    = 0x20;

    for (i = 0; i < bar->count; i++)
    {
        pd       = bar->items[i];
        pd->xco  = bar->xco;
        bar->xco += strlen((char *)pd->name) + 2;

        for (j = 0; j < pd->count; j++)
        {
            int16_t w = strlen((char *)pd->items[j]->name);
            if (w > pd->width) pd->width = w;
        }
    }
}

// -----------------------------------------------------------------------

void json_build_ui(void)
{
    uC_screen_t *scr = active_screen;

    if (scr_alloc(scr) != 0)
        json_error("Out of memory allocating screen");

    fix_windows(scr);

    if (scr->menu_bar != NULL)
        fix_menus(scr);
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
