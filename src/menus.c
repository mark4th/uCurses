// uCurses menus
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>

#include "h/list.h"
#include "h/tui.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------
// menu bars are added to the screen not individual windows

//  menu_bar_t contains a list of pulldown_t structures
//      pulldown_t    "name"     pulldown_t has a name and a list of items
//           list_t of menu_item_t
//      pulldown_t    "name"
//           list_t of menu_item_t
//      pulldown_t    "name"
//           list_t of menu_item_t

// -----------------------------------------------------------------------

menu_bar_t *new_menu_bar(screen_t *scr)
{
    menu_bar_t *bar = NULL;
    window_t *win;

    if(scr != NULL)
    {
        bar = calloc(1, sizeof(menu_bar_t));

        if(bar != NULL)
        {
            win = win_open(scr->width, 1);

            if(win != NULL)
            {
                bar->window = win;
                *(uint64_t *)&win->attrs[0]  = 0x60a0000000080;
                win->attrs[ATTR] |= WIN_LOCKED;
                win->screen = scr;

                scr->menu_bar = bar;

                *(uint64_t *)bar->attr_normal   = 0x60a0000000080;
                *(uint64_t *)bar->attr_selected = 0x6050000000080;
                *(uint64_t *)bar->attr_disabled = 0x0605000000008;
            }
            else
            {
                free(bar);
                bar = NULL;
            }
        }
    }

    return bar;
}

// -----------------------------------------------------------------------
// add a new pulldown to a menu bar

uint32_t new_pulldown(menu_bar_t *bar, char *name)
{
    uint32_t result = -1;

    pulldown_t *pd = calloc(1, sizeof(pulldown_t));

    if(pd != NULL)
    {
        result = 0;

        pd->name  = name;
        pd->xco   = bar->xco;
        bar->xco += utf8_strlen(name) + 1;

        *(uint64_t *)bar->attr_normal   = 0x60a0000000080;
        *(uint64_t *)bar->attr_selected = 0x6050000000080;
        *(uint64_t *)bar->attr_disabled = 0x0605000000008;
    }

    return result;
}

// -----------------------------------------------------------------------
// add a menu entry to a pulldown

uint32_t new_menu_item(menu_bar_t *bar, pulldown_t *pd, char *name,
    menu_fp_t fp, uint16_t shortcut)
{
    uint32_t result = -1;   // assume failure
    uint16_t width;

    if(pd != NULL)
    {
        menu_item_t *item = calloc(1, sizeof(menu_item_t));

        if(item != NULL)
        {
            result = 0;
            item->fp = fp;

            // keep track of which item in menu is widest as that will determine
            // the width of the menus oulldown window

            width = utf8_strlen(name);
            if(width > pd->width)
            {
                pd->width = width;
            }

            item->name     = name;
            item->fp       = fp;
            item->shortcut = shortcut;

            if((result = list_append_node(bar->items, item)) != 0)
            {
                free(item);  // oopts
            }
        }
    }

    return result;
}

// -----------------------------------------------------------------------

void bar_draw(screen_t *scr)
{
    uint16_t i = 0;
    uint8_t fg, bg;
    list_t *l;
    pulldown_t *pd;
    menu_bar_t *bar = scr->menu_bar;

    if(bar != NULL)
    {
        window_t *win = bar->window;
        if(win != NULL)
        {
            win_el(win);     // there can be only one!
            l = bar->items;

            while((pd = list_scan(l)) != NULL)
            {
                l = NULL;
                fg = (i == bar->which) ? pd->selected[FG] : pd->normal[FG];
                bg = (i == bar->which) ? pd->selected[BG] : pd->normal[BG];
                win_set_fg(win, fg);
                win_set_bg(win, bg);
                win_puts(win, pd->name);
                i++;
            }
        }
    }
}

// -----------------------------------------------------------------------

// =======================================================================
