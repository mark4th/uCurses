// menu_bar.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>
#include <string.h>

#include "uC_menus.h"
#include "uC_win_printf.h"
#include "uC_alloc.h"
#include "uC_attribs.h"
#include "uC_utils.h"
#include "uC_utf8.h"

// -----------------------------------------------------------------------

#ifdef UC_MENUS

extern uC_attribs_t uC_attrs_normal;
extern uC_attribs_t uC_attrs_selected;
extern uC_attribs_t uC_attrs_disabled;

// -----------------------------------------------------------------------
// set attriutes to draw next pulldown item with

// i    = menu index
// pd   = pointer to pulldown
// p    = pointer to attribute word
// item = menu item i points to within pulldown

static void pd_set_attr(int16_t i, pulldown_t *pd, uC_attribs_t *p,
    menu_item_t *item)
{
    if ((item->flags & uC_MENU_DISABLED) != 0)
    {
        *p = pd->disabled_attrs;
        return;
    }

    if (i == pd->which)
    {
        *p = pd->selected_attrs;
        return;
    }

    *p = pd->attrs;
}

// -----------------------------------------------------------------------

static int16_t pd_label_width(pulldown_t *pd)
{
    int16_t i;
    int16_t width = 0;

    for (i = 0; i < pd->count; i++)
    {
        int16_t item_width = uC_utf8_strlen(
            (uint8_t *)pd->items[i]->name);

        if (item_width > width)
        {
            width = item_width;
        }
    }

    return width;
}

// -----------------------------------------------------------------------

static void draw_item_shortcut(uC_window_t *win, menu_item_t *item,
    int16_t label_width)
{
    char shortcut[12];
    const char *p;

    if (!uC_menu_shortcut_display(item->shortcut, shortcut,
        sizeof(shortcut)) || win->width < 1)
    {
        return;
    }

    while ((win->cx <= label_width) && (win->cx != 0) &&
        (win->cx < win->width))
    {
        uC_win_emit(win, win->blank);
    }

    p = shortcut;
    while (*p && (win->cx < win->width) && (win->cx != 0))
    {
        uC_win_emit(win, *p++);
    }
}

// -----------------------------------------------------------------------
// draw menu items into pulldown window

void draw_pd(pulldown_t *pd)
{
    menu_item_t *item;
    uC_window_t *win;
    int16_t i;
    int16_t label_width;

    if ((pd != NULL) && (pd->count != 0) && (pd->window != NULL))
    {
        win = pd->window;
        label_width = pd_label_width(pd);

        for (i = 0; i != pd->count; i++)
        {
            item = pd->items[i];

            pd_set_attr(i, pd, &win->attrs, item);
            uC_win_cup(win, 0, i);
            uC_win_puts(win, item->name);
            draw_item_shortcut(win, item, label_width);

            // this ensures that the highlight on the selected menu item
            // spans the entire width of the pulldown window

            while ((win->cx != win->width) && (win->cx != 0))
            {
                uC_win_emit(win, win->blank);
            }
        }
    }
}

// -----------------------------------------------------------------------
// set attributes to draw menu bar item with

// i   = currently selectd meny bar item
// bar = pointer to menu bar structure
// p   = attribute chunk to draw with
// pd  = pointer to pulldown structure

static void bar_set_attrs(int16_t i, menu_bar_t *bar, uC_attribs_t *p,
    pulldown_t *pd)
{
    if ((i == bar->which) && (bar->active != 0))
    {
        *p = bar->selected_attrs;
        return;
    }

    if ((pd->flags & uC_MENU_DISABLED) != 0)
    {
        *p = bar->disabled_attrs;
        return;
    }

    *p = bar->attrs;
}

// -----------------------------------------------------------------------
// draws menu bar text, does not draw bar into screen

static void bar_draw_text(uC_screen_t *scr)
{
    int16_t i;
    pulldown_t *pd;
    uC_window_t *win;

    menu_bar_t *bar = scr->menu_bar;
    win = bar->window;

    if ((bar != NULL) && (win != NULL))
    {
        win->attrs = bar->attrs;
        uC_win_clear(win);
        uC_win_emit(win, win->blank);

        for (i = 0; i < bar->count; i++)
        {
            pd = bar->items[i];

            // set windows attributes according to the state of
            // this pulldown.  the pulldown may be active, inactive
            // or disabled.

            bar_set_attrs(i, bar, &win->attrs, pd);

            // todo: keyboard shortcut indicated by an underline?
            // same function as is used for button wigets?

            uC_win_printf(win, "%8%s%8", win->blank,
                pd->name, win->blank);
        }
    }
}

// -----------------------------------------------------------------------

static void pd_close(pulldown_t *pd)
{
    menu_item_t *item;

    if (pd != NULL)
    {
        while (pd->count-- != 0)
        {
            item = pd->items[pd->count];
            pd->items[pd->count] = NULL;
            menu_item_shortcut_remove(item);
            uC_ui_free(item);
        }

        // window only exists if this pulldown is currently active

        uC_win_close(pd->window);
        pd->window = NULL;
        uC_ui_free(pd);
    }
}

// -----------------------------------------------------------------------
// close all pulldowns atached to the bar

static void bar_close_pds(menu_bar_t *bar)
{
    pulldown_t *pd;

    while (bar->count-- != 0)
    {
        pd = bar->items[bar->count];
        bar->items[bar->count] = NULL;
        pd_close(pd);
    }
}

// -----------------------------------------------------------------------

API void uC_menu_bar_close(uC_screen_t *scr)
{
    menu_bar_t *bar;

    if (scr != NULL)
    {
        bar = scr->menu_bar;

        if (bar != NULL)
        {
            bar_close_pds(bar);
            uC_win_close(bar->window);
            uC_ui_free(bar);
            uC_menu_deinit_keys();
        }
        scr->menu_bar = NULL;
    }
}

/// -----------------------------------------------------------------------

static void init_bar(uC_screen_t *scr, uC_window_t *win, menu_bar_t *bar)
{
    bar->window = win;

    // make window non scrolling
    win->flags = uC_WIN_LOCKED;
    win->blank = 0x20;

    win->screen   = scr;
    scr->menu_bar = bar;

    bar->xco            = 2;
    bar->attrs          = uC_attrs_normal;
    bar->selected_attrs = uC_attrs_selected;
    bar->disabled_attrs = uC_attrs_disabled;
}

// -----------------------------------------------------------------------

API int32_t uC_menu_bar_open(uC_screen_t *scr)
{
    menu_bar_t *bar = NULL;
    uC_window_t *win;

    if (scr != NULL)
    {
        bar = uC_alloc(uC_MEM_ZONE_UI, sizeof(menu_bar_t));
        win = uC_win_open(scr->width, 1);

        if ((bar != NULL) && (win != NULL))
        {
            init_bar(scr, win, bar);
            uC_menu_init_keys();
            return 0;
        }
        uC_ui_free(bar);
        uC_ui_free(win);
    }

    return -1;
}

// -----------------------------------------------------------------------
// called in the screen update loop

void scr_update_menus(uC_screen_t *scr)
{
    menu_bar_t *bar;
    pulldown_t *pd;

    if (scr->menu_bar != NULL)
    {
        bar = scr->menu_bar;

        // draw all text into memu bar window then write that to
        // the screen buffer
        bar_draw_text(scr);
        scr_draw_win(bar->window);

        if (bar->active != 0)
        {
            pd = bar->items[bar->which];

            // draw all text inside pulldown memus window
            draw_pd(pd);
            // draw pulldown window into screen
            scr_draw_win((uC_window_t *)pd->window);
        }
    }
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
