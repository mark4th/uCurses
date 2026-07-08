// uC_pulldown.c  --  uCurses pulldown menus
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

#include "uC_menus.h"
#include "uC_utf8.h"
#include "uC_utils.h"
#include "uC_borders.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------

#ifdef UC_MENUS

extern uC_attribs_t uC_attrs_normal;
extern uC_attribs_t uC_attrs_selected;
extern uC_attribs_t uC_attrs_disabled;

// -----------------------------------------------------------------------
// find address of pulldown structure with specified name

static pulldown_t *pd_find(uC_screen_t *scr, const char *name)
{
    pulldown_t *pd = NULL;
    menu_bar_t *bar;
    int32_t name_hash;
    int32_t hash;

    int16_t i;

    if ((scr == NULL) || (scr->menu_bar == NULL) || (name == NULL))
    {
        return NULL;
    }

    bar = scr->menu_bar;
    name_hash = uC_fnv_hash((uint8_t *)name);

    for (i = 0; i < bar->count; i++)
    {
        pd = bar->items[i];

        if (pd)
        {
            hash = uC_fnv_hash((uint8_t *)pd->name);
            if (hash == name_hash)
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

static menu_item_t *item_find(pulldown_t *pd, const char *name)
{
    menu_item_t *item = NULL;
    int32_t name_hash;
    int32_t hash;
    int16_t i;

    if ((pd == NULL) || (name == NULL))
    {
        return NULL;
    }

    name_hash = uC_fnv_hash((uint8_t *)name);

    for (i = 0; i < pd->count; i++)
    {
        item = pd->items[i];

        if (item)
        {
            hash = uC_fnv_hash((uint8_t *)item->name);
            if (hash == name_hash)
            {
                break;
            }
        }
        item = NULL;
    }

    return item;
}

// -----------------------------------------------------------------------

API void uC_menu_pd_disable(uC_screen_t *scr, const char *name)
{
    pulldown_t *pd;

    if (scr)
    {
        pd = pd_find(scr, name);

        if (pd)
        {
            pd->flags |= uC_MENU_DISABLED;
            menu_pd_shortcuts_remove(pd);
            menu_normalize_selection(scr);
        }
    }
}

// -----------------------------------------------------------------------

API void uC_menu_pd_enable(uC_screen_t *scr, const char *name)
{
    pulldown_t *pd;

    if (scr)
    {
        pd = pd_find(scr, name);

        if (pd)
        {
            pd->flags &= ~uC_MENU_DISABLED;
            menu_pd_shortcuts_register(pd);
            menu_normalize_selection(scr);
        }
    }
}

// -----------------------------------------------------------------------

API void uC_menu_item_disable(uC_screen_t *scr, const char *pd_name,
    const char *item_name)
{
    pulldown_t *pd;
    menu_item_t *item;

    pd = pd_find(scr, pd_name);
    item = item_find(pd, item_name);

    if (item != NULL)
    {
        item->flags |= uC_MENU_DISABLED;
        menu_item_shortcut_remove(item);
        menu_normalize_selection(scr);
    }
}

// -----------------------------------------------------------------------

API void uC_menu_item_enable(uC_screen_t *scr, const char *pd_name,
    const char *item_name)
{
    pulldown_t *pd;
    menu_item_t *item;

    pd = pd_find(scr, pd_name);
    item = item_find(pd, item_name);

    if (item != NULL)
    {
        item->flags &= ~uC_MENU_DISABLED;
        menu_item_shortcut_register(pd, item);
        menu_normalize_selection(scr);
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

        win->flags = (uC_WIN_BOXED | uC_WIN_LOCKED);
        win->blank = 0x20;

        win->bdr_attrs.flags.bits =
            (uC_ATTR_FLAG_GRAY_FG | uC_ATTR_FLAG_GRAY_BG | uC_ATTR_FLAG_BOLD);

        win->bdr_attrs.fg = 11;
        win->bdr_attrs.bg = 4;

        // win->bdr_attrs.fg = pd->attrs.fg;
        // win->bdr_attrs.bg = pd->attrs.bg;

        win->border_type = uC_BDR_CURVED;

        win->screen = scr;
        pd->window  = win;

        result = 0;
    }

    return result;
}

// -----------------------------------------------------------------------
// add a new pulldown to a menu bar

// this seems to be surplus to requirements and Im not sure what I was
// thinking when I wrote it.  it is not referenced anywhere and i did not
// mark it as part of the public API... was I supposed to?

API int32_t uC_menu_new_pd(uC_screen_t *scr, const char *name)
{
    int32_t result = -1;

    menu_bar_t *bar = scr->menu_bar;
    pulldown_t *pd;

    if (bar && (bar->count != uC_MAX_MENU_ITEMS))
    {
        pd = uC_alloc(uC_MEM_ZONE_UI, sizeof(pulldown_t));

        if (pd)
        {
            bar->items[bar->count++] = pd;

            pd->name = name;
            pd->xco = bar->xco;

            bar->xco += uC_utf8_strlen((uint8_t *)name) + 2;

            pd->attrs          = uC_attrs_normal;
            pd->selected_attrs = uC_attrs_selected;
            pd->disabled_attrs = uC_attrs_disabled;

            result = 0;
        }
    }

    return result;
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
