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

static pulldown_t *pd_find(uC_screen_t *scr, uint8_t *name)
{
    pulldown_t *pd = NULL;
    menu_bar_t *bar = scr->menu_bar;
    int32_t name_hash = fnv_hash(name);
    int32_t hash;

    int16_t i;

    for (i = 0; i < bar->count; i++)
    {
        pd = bar->items[i];

        if (pd != NULL)
        {
            hash = fnv_hash(pd->name);
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

API void uC_menu_pd_disable(uC_screen_t *scr, uint8_t *name)
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

API void uC_menu_pd_enable(uC_screen_t *scr, uint8_t *name)
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

        win->flags = (WIN_BOXED | WIN_LOCKED);
        win->blank = 0x20;

        win->bdr_attrs.flags.bits =
            (ATTR_FLAG_GRAY_FG | ATTR_FLAG_GRAY_BG | ATTR_FLAG_BOLD);

        win->bdr_attrs.fg = 11;
        win->bdr_attrs.bg = 4;

        // win->bdr_attrs.fg = pd->attrs.fg;
        // win->bdr_attrs.bg = pd->attrs.bg;

        win->border_type = BDR_CURVED;

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

API int32_t uC_menu_new_pd(uC_screen_t *scr, uint8_t *name)
{
    int32_t result = -1;

    menu_bar_t *bar = scr->menu_bar;
    pulldown_t *pd;

    if ((bar != NULL) && (bar->count != MAX_MENU_ITEMS))
    {
        pd = uC_alloc(uC_MEM_ZONE_UI, sizeof(pulldown_t));

        if (pd != NULL)
        {
            bar->items[bar->count++] = pd;

            pd->name = name;
            pd->xco = bar->xco;

            bar->xco += uC_utf8_strlen(name) + 2;

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
