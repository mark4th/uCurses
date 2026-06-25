// uCurses menus
// -----------------------------------------------------------------------

#include <inttypes.h>

#include "uC_menus.h"
#include "uC_keys.h"
#include "uC_utf8.h"
#include "uC_alloc.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

#ifdef UC_MENUS

// -----------------------------------------------------------------------

static void menu_shortcut_action(void *context)
{
    menu_item_t *item = context;

    if ((item != NULL) && ((item->flags & uC_MENU_DISABLED) == 0) &&
        (item->parent != NULL) &&
        ((item->parent->flags & uC_MENU_DISABLED) == 0) &&
        (item->fp != NULL))
    {
        uC_menu_close(item->screen);
        item->fp();
    }
}

// -----------------------------------------------------------------------

bool menu_item_shortcut_register(pulldown_t *pd, menu_item_t *item)
{
    if ((pd == NULL) || (item == NULL) || (item->screen == NULL) ||
        (item->shortcut == 0) || item->shortcut_registered ||
        ((pd->flags & uC_MENU_DISABLED) != 0) ||
        ((item->flags & uC_MENU_DISABLED) != 0))
    {
        return false;
    }

    item->shortcut_registered = uC_shortcut_register(item->screen,
        item->shortcut, menu_shortcut_action, item, item);

    return item->shortcut_registered;
}

// -----------------------------------------------------------------------

void menu_item_shortcut_remove(menu_item_t *item)
{
    if ((item != NULL) && item->shortcut_registered)
    {
        uC_shortcut_remove_owner(item->screen, item);
        item->shortcut_registered = false;
    }
}

// -----------------------------------------------------------------------

void menu_pd_shortcuts_register(pulldown_t *pd)
{
    int16_t i;

    if (pd == NULL)
    {
        return;
    }

    for (i = 0; i < pd->count; i++)
    {
        menu_item_shortcut_register(pd, pd->items[i]);
    }
}

// -----------------------------------------------------------------------

void menu_pd_shortcuts_remove(pulldown_t *pd)
{
    int16_t i;

    if (pd == NULL)
    {
        return;
    }

    for (i = 0; i < pd->count; i++)
    {
        menu_item_shortcut_remove(pd->items[i]);
    }
}

// -----------------------------------------------------------------------

static void init_item(uC_screen_t *scr, pulldown_t *pd, menu_item_t *item,
    const char *name, menu_fp_t fp, uC_shortcut_t shortcut)
{
    int16_t width;
    uint16_t shortcut_width = 0;
    int16_t i;

    pd->items[pd->count++] = item;

    item->name     = name;
    item->fp       = fp;
    item->shortcut = shortcut;
    item->screen   = scr;
    item->parent   = pd;

    // keep track of the widest displayed menu row.  If any item has a
    // shortcut, reserve one blank column and enough columns for the widest
    // shortcut label after the longest item label.

    width = 0;
    for (i = 0; i < pd->count; i++)
    {
        int16_t item_width = uC_utf8_strlen(
            (uint8_t *)pd->items[i]->name);
        uint16_t item_shortcut_width =
            uC_menu_shortcut_width(pd->items[i]->shortcut);

        if (item_width > width)
        {
            width = item_width;
        }
        if (item_shortcut_width > shortcut_width)
        {
            shortcut_width = item_shortcut_width;
        }
    }

    if (shortcut_width != 0)
    {
        width += 1 + (int16_t)shortcut_width;
    }
    pd->width = width;
}

// -----------------------------------------------------------------------

static int32_t new_item(uC_screen_t *scr, pulldown_t *pd, const char *name,
    menu_fp_t fp, uC_shortcut_t shortcut)
{
    int32_t result = -1;

    if (pd->count != uC_MAX_MENU_ITEMS)
    {
        menu_item_t *item = uC_alloc(uC_MEM_ZONE_UI,
            sizeof(menu_item_t));

        if (item)
        {
            bool shortcut_live = (shortcut != 0) &&
                ((pd->flags & uC_MENU_DISABLED) == 0);

            init_item(scr, pd, item, name, fp, shortcut);

            if (!shortcut_live || menu_item_shortcut_register(pd, item))
            {
                result = 0;
            }
            else
            {
                if (pd->count > 0)
                {
                    pd->count--;
                    pd->items[pd->count] = NULL;
                }
                uC_ui_free(item);
            }
        }
    }

    return result;
}

// -----------------------------------------------------------------------
// add an entry to a pulldown

API int32_t uC_menu_new_item(uC_screen_t *scr, const char *name,
    menu_fp_t fp, uC_shortcut_t shortcut)
{
    int32_t result = -1;    // assume failure

    pulldown_t *pd;
    menu_bar_t *bar;

    if (scr && scr->menu_bar)
    {
        bar = scr->menu_bar;
        pd  = bar->items[bar->count - 1];

        if (pd)
        {
            result = new_item(scr, pd, name, fp, shortcut);
        }
    }

    return result;
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
