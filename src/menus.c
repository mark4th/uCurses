// uCurses menus
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>

#include "h/list.h"
#include "h/tui.h"
#include "h/uCurses.h"

extern uint8_t attrs[8];

// -----------------------------------------------------------------------
// hard coded for now

#define NORMAL   0x4020000000080
#define SELECTED 0x1060000000080
#define DISABLED 0x0605000000008

// -----------------------------------------------------------------------
// menu bars are added to the screen not individual windows

// menu bar
//    pulldown
//       item
//       item
//       item
//    pulldown
//       ...

// -----------------------------------------------------------------------

#include <stdio.h>

uint32_t bar_open(screen_t *scr)
{
    menu_bar_t *bar = NULL;
    window_t *win;

    uint32_t result = -1;

    if(scr != NULL)
    {
        bar = calloc(1, sizeof(menu_bar_t));
        win = win_open(scr->width, 1);

        if((bar != NULL) && (win != NULL))
        {
            bar->window    = win;
            // make window non scrolling

            win->attrs[ATTR] |= WIN_LOCKED;

            win->screen = scr;
            scr->menu_bar = bar;

            *(uint64_t *)bar->normal   = NORMAL;
            *(uint64_t *)bar->selected = SELECTED;
            *(uint64_t *)bar->disabled = DISABLED;

            result = 0;
        }
        else
        {
            free(bar);
            free(win);
        }
    }

    return result;
}

// -----------------------------------------------------------------------
// add a new pulldown to a menu bar

uint32_t new_pulldown(screen_t *scr, char *name)
{
    uint32_t result = -1;
    menu_bar_t *bar = scr->menu_bar;

    if(bar != NULL)
    {
        pulldown_t *pd = calloc(1, sizeof(pulldown_t));

        if(pd != NULL)
        {
            if((result = list_append_node(&bar->items, pd)) != 0)
            {
                free(pd);
            }
            else
            {
                pd->which = -1;
                pd->name  = name;
                pd->xco   = bar->xco;
                bar->xco += utf8_strlen(name) + 1;

                *(uint64_t *)pd->normal   = NORMAL;
                *(uint64_t *)pd->selected = SELECTED;
                *(uint64_t *)pd->disabled = DISABLED;

                // once a pulldown is added to a bar you must populate
                // it completely before adding another

                bar->pd   = pd;
            }
        }
    }

    return result;
}

// -----------------------------------------------------------------------

static uint32_t new_item(pulldown_t *pd, char *name,
    menu_fp_t fp, uint16_t shortcut)
{
    uint16_t width;
    uint32_t result = -1;

    menu_item_t *item = calloc(1, sizeof(menu_item_t));

    if(item != NULL)
    {
        if((result = list_append_node(&pd->items, item)) != 0)
        {
            free(item);  // oopts
        }
        else
        {
            item->fp = fp;

            // keep track of which item in menu is widest as that will
            // determine the width of the pulldown window

            width = utf8_strlen(name);
            if(width > pd->width)
            {
                pd->width = width;
            }

            item->name     = name;
            item->fp       = fp;
            item->shortcut = shortcut;
        }
    }

    return result;
}

// -----------------------------------------------------------------------
// add an entry to a pulldown

uint32_t new_menu_item(screen_t *scr, char *name, menu_fp_t fp,
    uint16_t shortcut)
{
    pulldown_t *pd;
    menu_bar_t *bar;
    uint32_t result = -1;   // assume failure

    if((scr != NULL) && (scr->menu_bar != NULL))
    {
        bar = scr->menu_bar;
        pd  = bar->pd;

        if(pd != NULL)
        {
            result = new_item(pd, name, fp, shortcut);
        }
    }

    return result;
}

// -----------------------------------------------------------------------
// draws menu bar text, does not draw bar into screen

void bar_draw_text(screen_t *scr)
{
    uint16_t i = 0;
    list_t *l;
    pulldown_t *pd;
    window_t *win;

    menu_bar_t *bar = scr->menu_bar;

    if((bar != NULL) && (bar->window != NULL))
    {
        win = bar->window;
        *(uint64_t *)&win->attrs[0] = *(uint64_t *)&bar->normal[0];
        win_clear(win);
        win->cx = 1;

        l = &bar->items;

        while((pd = list_scan(l)) != NULL)
        {
            l = NULL;
            *(uint64_t *)&win->attrs[0] = (i == bar->which)
                 ? *(uint64_t *)pd->selected
                 : *(uint64_t *)pd->normal;
            win_emit(win, win->blank);
            win_puts(win, pd->name);
            win_emit(win, win->blank);
            i++;
        }
    }
}

// -----------------------------------------------------------------------

void bar_close(screen_t *scr)
{
    menu_bar_t *bar = scr->menu_bar;
    pulldown_t *pd;
    menu_item_t *item;

    while((pd = list_pop(&bar->items)) != 0)
    {
        while((item = list_pop(&pd->items)) != 0)
        {
            free(item);
        }
        free(pd);
    }
    win_close(bar->window);
    free(bar);
}

// =======================================================================
