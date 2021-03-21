// uCurses menus
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>
//#include <stdio.h>

#include "h/list.h"
#include "h/tui.h"
#include "h/uCurses.h"

extern uint8_t attrs[8];
extern screen_t *active_screen;

// -----------------------------------------------------------------------
// hard coded attributes for now

#define NORMAL   0x04030000000080
#define SELECTED 0x01060000000080
#define DISABLED 0x080400000000c2

// -----------------------------------------------------------------------

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
            bar->window = win;

            // make window non scrolling
            win->attrs[ATTR] |= WIN_LOCKED;

            win->screen   = scr;
            scr->menu_bar = bar;

// win_set_gray_fg(win, 8);
// win_set_gray_bg(win, 4);
// printf("%lx\n", *(long unsigned int *)&win->attrs[0]);
// exit(0);

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

static uint32_t new_pull(menu_bar_t *bar, pulldown_t *pd, char *name)
{
    uint32_t result = -1;

    if(bar->count != MAX_MENU_ITEMS)
    {
        bar->items[bar->count++] = pd;

        pd->which = -1;
        pd->name  = name;
        pd->xco   = bar->xco;
        bar->xco += utf8_strlen(name) + 2;

        *(uint64_t *)pd->normal   = NORMAL;
        *(uint64_t *)pd->selected = SELECTED;
        *(uint64_t *)pd->disabled = DISABLED;

        result = 0;
    }
    else
    {
        free(pd);
    }

    return result;
}

// -----------------------------------------------------------------------
// add a new pulldown to a menu bar

uint32_t new_pulldown(screen_t *scr, char *name)
{
    uint32_t result = -1;
    menu_bar_t *bar = scr->menu_bar;
    pulldown_t *pd;

    if(bar != NULL)
    {
        if((pd = calloc(1, sizeof(pulldown_t))) != NULL)
        {
            result = new_pull(bar, pd, name);
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

    if(pd->count != MAX_MENU_ITEMS)
    {
        menu_item_t *item = calloc(1, sizeof(menu_item_t));

        if(item != NULL)
        {
            pd->items[pd->count++] = item;

            item->name     = name;
            item->fp       = fp;
            item->shortcut = shortcut;

            // keep track of which item in menu is widest as that will
            // determine the width of the pulldown window

            width = utf8_strlen(name);

            if(width > pd->width)
            {
                pd->width = width;
            }
        }
    }
    return result;
}

// -----------------------------------------------------------------------
// add an entry to a pulldown

uint32_t new_menu_item(screen_t *scr, char *name, menu_fp_t fp,
    uint16_t shortcut)
{
    uint32_t result = -1;   // assume failure

    pulldown_t *pd;
    menu_bar_t *bar;

    if((scr != NULL) && (scr->menu_bar != NULL))
    {
        bar = scr->menu_bar;
        pd  = bar->items[bar->count -1];

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
    pulldown_t *pd;
    window_t *win;

    menu_bar_t *bar = scr->menu_bar;

    if((bar != NULL) && (bar->window != NULL))
    {
        win = bar->window;
        *(uint64_t *)&win->attrs[0] = *(uint64_t *)&bar->normal[0];
        win_clear(win);
        win_crsr_rt(win);

        for(i = 0; i < bar->count; i++)
        {
            pd = bar->items[i];

            // }:) trust me, im a professional! }:)

            *(uint64_t *)&win->attrs[0] =
                ((i == bar->which) && (bar->active != 0))
                     ? *(uint64_t *)pd->selected
                     : (pd->flags & MENU_DISABLED)
                         ? *(uint64_t *)pd->disabled
                         : *(uint64_t *)pd->normal;

            win_emit(win, win->blank);
            win_puts(win, pd->name);
            win_emit(win, win->blank);
        }
    }
}

// -----------------------------------------------------------------------

void bar_close(screen_t *scr)
{
    menu_bar_t *bar = scr->menu_bar;
    pulldown_t *pd;
    uint16_t i, j;

    for(i = 0; i != bar->count; i++)
    {
        pd = bar->items[i];
        for(j = 0; j != pd->count; j++)
        {
            free(pd->items[j]);
            pd->items[j] = NULL;
        }
        free(pd);
    }

    win_close(bar->window);
    free(bar);
}

// -----------------------------------------------------------------------
// find address of pulldown structure with specified name

pulldown_t *pd_find(screen_t *scr, char *name)
{
    pulldown_t *pd = NULL;
    menu_bar_t *bar = scr->menu_bar;
    uint16_t i;
    uint16_t len = utf8_strlen(name);

    for(i = 0; i < bar->count; i++)
    {
        pd = bar->items[i];

        if(utf8_strncmp(name, pd->name, len) == 0)
        {
            break;
        }
    }
    return pd;
}

// -----------------------------------------------------------------------

void pd_disable(screen_t *scr, char *name)
{
    pulldown_t *pd;

    if(scr != NULL)
    {
        pd = pd_find(scr, name);

        if(pd != NULL)
        {
            pd->flags |= MENU_DISABLED;
        }
    }
}

// -----------------------------------------------------------------------

void pd_enable(screen_t *scr, char *name)
{
    pulldown_t *pd;

    if(scr != NULL)
    {
        pd = pd_find(scr, name);

        if(pd != NULL)
        {
            pd->flags &= MENU_DISABLED;
        }
    }
}

// -----------------------------------------------------------------------

static void menu_up(void)    { }
static void menu_down(void)  { }

// -----------------------------------------------------------------------

static void menu_activate(void)
{
   menu_bar_t *bar = active_screen->menu_bar;
   bar->active ^= 1;
}

// -----------------------------------------------------------------------

static void next_pd(menu_bar_t *bar)
{
    if(bar->which != bar->count -1)
    {
        bar->which++;
    }
    else
    {
        bar->which = 0;
    }
}

// -----------------------------------------------------------------------

static void menu_right(void)
{
    menu_bar_t *bar = active_screen->menu_bar;
    pulldown_t *pd;
    uint16_t n;

    if((bar != NULL) && (bar->active != 0))
    {
        n = bar->count;

        while(n != 0)
        {
            next_pd(bar);
            n--;
            pd = bar->items[bar->which];
            if((pd->flags & MENU_DISABLED) == 0)
            {
                n = 0;
            }
        }
    }
}

// -----------------------------------------------------------------------

static void prev_pd(menu_bar_t *bar)
{
    if(bar->which != 0)
    {
        bar->which--;
    }
    else
    {
        bar->which = bar->count -1;
    }
}

// -----------------------------------------------------------------------

static void menu_left(void)
{
    menu_bar_t *bar = active_screen->menu_bar;
    pulldown_t *pd;
    uint16_t n;

    if((bar != NULL) && (bar->active != 0))
    {
        n = bar->count;

        while(n != 0)
        {
            prev_pd(bar);
            n--;
            pd = bar->items[bar->which];
            if((pd->flags & MENU_DISABLED) == 0)
            {
                n = 0;
            }
        }
    }
}

// -----------------------------------------------------------------------
// you should not modify these pointers... but i can! (tm)

void menu_init(void)
{
    init_key_handlers();
    set_key_action(K_CUU1, menu_up);
    set_key_action(K_CUD1, menu_down);
    set_key_action(K_CUB1, menu_left);
    set_key_action(K_CUF1, menu_right);
    set_key_action(K_F10, menu_activate);
    // without this the cursor up and down keys can return escape
    // sequences that do not match what is specified for them in
    // the terminfo file - emitting an smkx escape sequence to the
    // terminal turns on keyboard transmit mode what ever that is
    ti_smkx();
}

// =======================================================================
