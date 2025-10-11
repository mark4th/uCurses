// uCurses menus
// -----------------------------------------------------------------------

#include <inttypes.h>

#include "uC_menus.h"
#include "uC_utf8.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------

#ifdef UC_MENUS

// -----------------------------------------------------------------------

static void init_item(pulldown_t *pd, menu_item_t *item,
    char *name, menu_fp_t fp, uint16_t shortcut)
{
    int16_t width;

    pd->items[pd->count++] = item;

    item->name     = name;
    item->fp       = fp;
    item->shortcut = shortcut;

    // keep track of which item in menu is widest as that will
    // determine the width of the pulldown window

    width = uC_utf8_strlen(name);

    if (width > pd->width)
    {
        pd->width = width;
    }
}

// -----------------------------------------------------------------------

static int32_t new_item(pulldown_t *pd, char *name, menu_fp_t fp,
    uint16_t shortcut)
{
    int32_t result = -1;

    if (pd->count != MAX_MENU_ITEMS)
    {
        menu_item_t *item = uC_alloc(uC_MEM_ZONE_UI, sizeof(menu_item_t));

        if (item != NULL)
        {
            init_item(pd, item, name, fp, shortcut);
            result = 0;
        }
    }

    return result;
}

// -----------------------------------------------------------------------
// add an entry to a pulldown

API int32_t uC_menu_new_item(uC_screen_t *scr, char *name, menu_fp_t fp,
    int16_t shortcut)
{
    int32_t result = -1;    // assume failure

    pulldown_t *pd;
    menu_bar_t *bar;

    if ((scr != NULL) && (scr->menu_bar != NULL))
    {
        bar = scr->menu_bar;
        pd  = bar->items[bar->count - 1];

        if (pd != NULL)
        {
            result = new_item(pd, name, fp, shortcut);
        }
    }

    return result;
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
