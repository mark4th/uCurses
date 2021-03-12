// uCurses menus
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>

#include "h/list.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

typedef void (*menu_fp_t)(void);

// -----------------------------------------------------------------------

typedef struct
{
    list_t list;            // linked list of entries
    uint8_t *name;
    menu_fp_t *fp;          // function to execute
    uint16_t flags;         // masks for enabled/disabled etc
    uint16_t count;         // number of items in menu
    uint16_t which;         // current selected item
    uint16_t shortcut;      // keyboard shortcut
    uint16_t xco;           // x coordinate of menu window
    uint8_t normal[8];      // attribs for non selected menu items
    uint8_t selected[8];    // atrribs for selected menu item
    uint8_t disabled[8];    // attribs for disabled meny items
} menu_item_t;

// -----------------------------------------------------------------------

typedef struct
{
    screen_t *screen;         // menu bars parent screen
    list_t *items;            // first itme is menu bar name
    uint16_t count;           // number of pulldowns in this bar
    uint16_t which;           // which pulldown item is active
    uint16_t width;           // width of widest element in menu bar
    uint8_t attr_normal[8];   // attribs for non selected menu bar items
    uint8_t attr_selected[8]; // attribs for selected menu bar items
    uint8_t attr_disabled[8]; // attribs for disabled menu bar items
    uint8_t attr_border[8];   // attribs for pulldown menu border
} menu_bar_t;

// -----------------------------------------------------------------------

menu_bar_t *new_menu_bar(screen_t *scr)
{
    menu_bar_t *bar = calloc(1, sizeof(menu_bar_t));
    bar->screen = scr;
    scr->menu_bar = bar;

    *(uint64_t *)bar->attr_normal   = 0x60a0000000080;
    *(uint64_t *)bar->attr_selected = 0x6050000000080;
    *(uint64_t &)bar->attr_disabled = 0x0605000000008;

    return bar;
}

// -----------------------------------------------------------------------

void *new_menu_item(menu_bar_t *bar, char *name, fp_t fp)
{
    menu_item_t *item = calloc(1, sizeof(menu_item_t));
    item->fp = fp;

    width =
    *(uint64_t *)bar->attr_normal   = 0x60a0000000080;
    *(uint64_t *)bar->attr_selected = 0x6050000000080;
    *(uint64_t &)bar->attr_disabled = 0x0605000000008;

    item->name = name;
    item->fp = fp;

    if(list_append_node(&bar->items, item) != 0)
    {
        // oopts
    }
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

// =======================================================================
