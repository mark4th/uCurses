// uCurses menus
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>

#include "h/list.h"
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

typedef void (*menu_fp_t)(void);

// -----------------------------------------------------------------------

typedef struct
{
    list_t list;            // linked list of entries
    uint8_t *name;
    menu_fp_t *fp;          // function to execute
    uint16_t shortcut;      // keyboard shortcut
} menu_item_t;

// -----------------------------------------------------------------------

typedef struct
{
    char *name;             // menu bar name for this pulldown menu
    uint16_t width;         // width of widest item in pulldown menu
    list_t items;           // list of items in this pulldown
    uint8_t attr[8];        // attribs for pulldown menu border
    uint16_t flags;         // masks for enabled/disabled etc
    uint16_t which;         // current selected item
    uint16_t xco;           // x coordinate of menu window
    uint8_t normal[8];      // attribs for non selected menu items
    uint8_t selected[8];    // atrribs for selected menu item
    uint8_t disabled[8];    // attribs for disabled meny items
} pulldown_t;

// -----------------------------------------------------------------------

typedef struct
{
    screen_t *screen;         // menu bars parent screen
    list_t *items;            // list of pulldown menus in this bar
    uint16_t which;           // which pulldown item is active
    uint8_t attr_normal[8];   // attribs for non selected menu bar items
    uint8_t attr_selected[8]; // attribs for selected menu bar items
    uint8_t attr_disabled[8]; // attribs for disabled menu bar items
} menu_bar_t;

// -----------------------------------------------------------------------

menu_bar_t *new_menu_bar(screen_t *scr)
{
    menu_bar_t *bar = NULL;

    if(scr != NULL)
    {
        bar = calloc(1, sizeof(menu_bar_t));

        if(bar != NULL)
        {
            bar->screen   = scr;
            scr->menu_bar = bar;
            bar->xco = 1;
            *(uint64_t *)bar->attr_normal   = 0x60a0000000080;
            *(uint64_t *)bar->attr_selected = 0x6050000000080;
            *(uint64_t &)bar->attr_disabled = 0x0605000000008;
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

        pd->name = name;
        pd->xco = bar->xco;
        bar->xco += utf8_strlen(name);

        *(uint64_t *)bar->attr_normal   = 0x60a0000000080;
        *(uint64_t *)bar->attr_selected = 0x6050000000080;
        *(uint64_t &)bar->attr_disabled = 0x0605000000008;
    }
}

// -----------------------------------------------------------------------
// add a menu entry to a pulldown

uint32_t new_menu_item(pulldown_t *pd, char *name, menu_fp_t fp)
{
    uint32_t result = -1;   // assume failure

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
            if(width > pd.width)
            {
                pd->width = width;
            }

            item->name = name;
            item->fp = fp;

            if(result = list_append_node(&bar->items, item) != 0)
            {
                free(item);  // oopts
            }
        }
    }

    return result;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

// =======================================================================
