// menu_bar.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>

#include "menus.h"
#include "win_printf.h"

// -----------------------------------------------------------------------
// set attriutes to draw next pulldown item with

static void pd_set_attr(int16_t i, pulldown_t *pd, uint64_t *p,
    menu_item_t *item)
{
    // currently selected item?
    if (i == pd->which)
    {
        *p = pd->attr_grp.selected.chunk;
        return;
    }
    *p = ((item->flags & MENU_DISABLED) != 0)
        ? pd->attr_grp.disabled.chunk
        : pd->attr_grp.attrs.chunk;
}

// -----------------------------------------------------------------------
// populate pulldown window with menu items

void bar_populdate_pd(pulldown_t *pd)
{
    menu_item_t *item;
    window_t *win;
    int16_t i;

    if ((pd != NULL) && (pd->count != 0) && (pd->window != NULL))
    {
        win = pd->window;
        uC_win_clear(win);

        for (i = 0; i != pd->count; i++)
        {
            item = pd->items[i];

            pd_set_attr(i, pd, &win->attr_grp.attrs.chunk, item);

            uC_win_cup(win, 0, i);
            uC_win_puts(win, item->name);

            while ((win->cx != win->width) && (win->cx != 0))
            {
                uC_win_emit(win, win->blank);
            }
        }
    }
}

// -----------------------------------------------------------------------

static void bar_set_attrs(int16_t i, menu_bar_t *bar, uint64_t *p,
    pulldown_t *pd)
{
    if ((i == bar->which) && (bar->active != 0))
    {
        *p = bar->attr_grp.selected.chunk;
        return;
    }
    *p = ((pd->flags & MENU_DISABLED) != 0)
        ? bar->attr_grp.disabled.chunk
        : bar->attr_grp.attrs.chunk;
}

// -----------------------------------------------------------------------
// draws menu bar text, does not draw bar into screen

API void uC_bar_draw_text(screen_t *scr)
{
    int16_t i;
    pulldown_t *pd;
    window_t *win;

    menu_bar_t *bar = scr->menu_bar;
    win = bar->window;

    if ((bar != NULL) && (win != NULL))
    {
        win->attr_grp.attrs.chunk = bar->attr_grp.attrs.chunk;

        uC_win_clear(win);
        uC_win_emit(win, win->blank);

        for (i = 0; i < bar->count; i++)
        {
            pd = bar->items[i];

            bar_set_attrs(i, bar, &win->attr_grp.attrs.chunk, pd);

            uC_win_emit(win, win->blank);
            uC_win_puts(win, pd->name);
            uC_win_emit(win, win->blank);
        }
    }
}

// -----------------------------------------------------------------------

static void pd_close(pulldown_t *pd)
{
    if (pd != NULL)
    {
        while (pd->count-- != 0)
        {
            free(pd->items[pd->count]);
        }

        // window only exists if this pulldown is currently active
        uC_win_close(pd->window);
        pd->window = NULL;
        free(pd);
    }
}

// -----------------------------------------------------------------------
// cloase all pulldowns atached to the bar

static void bar_close_pds(menu_bar_t *bar)
{
    pulldown_t *pd;

    while (bar->count-- != 0)
    {
        pd = bar->items[bar->count];
        pd_close(pd);
    }
}

// -----------------------------------------------------------------------

API void uC_bar_close(screen_t *scr)
{
    menu_bar_t *bar;

    if (scr != NULL)
    {
        bar = scr->menu_bar;

        if (bar != NULL)
        {
            bar_close_pds(bar);
            uC_win_close(bar->window);
            bar->window = NULL;
            free_status();
            free(bar);
            scr->menu_bar = NULL;
        }
    }
}

/// -----------------------------------------------------------------------

static void init_bar(screen_t *scr, window_t *win, menu_bar_t *bar)
{
    bar->window = win;

    // make window non scrolling
    win->flags = WIN_LOCKED;
    win->blank = 0x20;

    win->screen   = scr;
    scr->menu_bar = bar;

    bar->attr_grp.attrs.chunk    = ATTRS_NORMAL;
    bar->attr_grp.selected.chunk = ATTRS_SELECTED;
    bar->attr_grp.disabled.chunk = ATTRS_DISABLED;

    bar->xco = 2;           // x coordinate of first pulldown menu window
}

// -----------------------------------------------------------------------

API int32_t uC_bar_open(screen_t *scr)
{
    menu_bar_t *bar = NULL;
    window_t *win;

    if (scr != NULL)
    {
        bar = calloc(1, sizeof(menu_bar_t));
        win = uC_win_open(scr->width, 1);

        if ((bar != NULL) && (win != NULL))
        {
            init_bar(scr, win, bar);
            return 0;
        }
        free(bar);
        free(win);
    }

    return -1;
}

// =======================================================================
