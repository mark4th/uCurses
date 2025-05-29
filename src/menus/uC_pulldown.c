// uC_pulldown.c  --  uCurses pulldown menus
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>

#include "uC_menus.h"
#include "uC_utf8.h"
#include "uC_utils.h"
#include "uC_borders.h"

// -----------------------------------------------------------------------
// find address of pulldown structure with specified name

static pulldown_t *pd_find(uC_screen_t *scr, char *name)
{
    pulldown_t *pd = NULL;
    menu_bar_t *bar = scr->menu_bar;
    int32_t name_hash = fnv_hash(name);

    int16_t i;

    for (i = 0; i < bar->count; i++)
    {
        pd = bar->items[i];

        if (pd != NULL)
        {
            if (fnv_hash(pd->name) == name_hash)
            {
                break;
            }
        }
        // leaves pd = NULL if we reached the end of the loop
        pd = NULL;
    }

    return pd;
}

// -----------------------------------------------------------------------

void pd_disable(uC_screen_t *scr, char *name)
{
    pulldown_t *pd;

    if (scr != NULL)
    {
        pd = pd_find(scr, name);

        if (pd != NULL)
        {
            pd->flags |= MENU_DISABLED;
        }
    }
}

// -----------------------------------------------------------------------

void pd_enable(uC_screen_t *scr, char *name)
{
    pulldown_t *pd;

    if (scr != NULL)
    {
        pd = pd_find(scr, name);

        if (pd != NULL)
        {
            pd->flags &= ~MENU_DISABLED;
        }
    }
}

// -----------------------------------------------------------------------
// create a new window for the pulldown menu we are about to display

// pulldowns have no associated window until they become active

int32_t bar_create_pd_win(uC_screen_t *scr, pulldown_t *pd)
{
    uC_window_t *win;
    int32_t result = -1;

    win = uC_win_open(pd->width, pd->count);

    if (win != NULL)
    {
        win->xco = pd->xco;
        win->yco = 2;

        win->flags = WIN_BOXED | WIN_LOCKED;
        win->blank = 0x20;

        win->bdr_attrs.bytes[ATTR] = (FG_GRAY | BG_GRAY | BOLD);
    win->bdr_attrs.bytes[FG] = 11;
    win->bdr_attrs.bytes[BG] = 4;
    // win->bdr_attrs.bytes[FG] = pd->attrs.bytes[FG];
    // win->bdr_attrs.bytes[BG] = pd->attrs.bytes[BG];

        win->border_type = BDR_CURVED;

        win->screen = scr;
        pd->window  = win;

        result = 0;
    }

    return result;
}

// -----------------------------------------------------------------------
// add a new pulldown to a menu bar

int32_t new_pulldown(uC_screen_t *scr, char *name)
{
    int32_t result = -1;
    menu_bar_t *bar = scr->menu_bar;
    pulldown_t *pd;

    if ((bar != NULL) && (bar->count != MAX_MENU_ITEMS))
    {
        pd = calloc(1, sizeof(pulldown_t));

        if (pd != NULL)
        {
            bar->items[bar->count++] = pd;

            pd->name = name;
            pd->xco = bar->xco;

            bar->xco += uC_utf8_strlen(name) + 2;

            pd->attrs.chunk          = uC_ATTRS_NORMAL;
            pd->selected_attrs.chunk = uC_ATTRS_SELECTED;
            pd->disabled_attrs.chunk = uC_ATTRS_DISABLED;

            result = 0;
        }
    }

    return result;
}

// =======================================================================
