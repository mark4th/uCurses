// menu_key.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

#include "uC_menus.h"
#include "uC_keys.h"
#include "uC_terminfo.h"

// -----------------------------------------------------------------------

extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------

static void redraw_pulldown(menu_bar_t *bar)
{
    pulldown_t *pd = bar->items[bar->which];

    if (pd->count != 0)
    {
        if (bar_create_pd_win(active_screen, pd) != 0)
        {
            bar_populdate_pd(pd);
        }
    }
}

// -----------------------------------------------------------------------

static void menu_activate(void)
{
    menu_bar_t *bar = active_screen->menu_bar;
    pulldown_t *pd;

    bar->active ^= 1;
    pd = bar->items[bar->which];

    if (bar->active != 0)
    {
        redraw_pulldown(bar);
        return;
    }
    uC_win_close(pd->window);
    pd->window = NULL;
}

// -----------------------------------------------------------------------

static void to_prev_menu_item(pulldown_t *pd)
{
    if (pd->which == 0)
    {
        pd->which = pd->count;
    }
    pd->which--;
}

// -----------------------------------------------------------------------

static void to_next_menu_item(pulldown_t *pd)
{
    pd->which = (pd->which != pd->count - 1)
        ? pd->which + 1
        : 0;
}

// -----------------------------------------------------------------------

static void next_pd(menu_bar_t *bar)
{
    bar->which = (bar->which != bar->count - 1)
        ? bar->which + 1
        : 0;
}

// -----------------------------------------------------------------------

static void prev_pd(menu_bar_t *bar)
{
    if (bar->which == 0)
    {
        bar->which = bar->count;
    }

    bar->which--;
}

// -----------------------------------------------------------------------

static void menu_up_down(int dir)
{
    menu_bar_t *bar = active_screen->menu_bar;
    pulldown_t *pd;
    int16_t n;
    menu_item_t *item;

    if ((bar != NULL) && (bar->active != 0))
    {
        pd = bar->items[bar->which];
        n  = bar->count;

        while (n != 0)
        {
            (dir > 0)
                ? to_prev_menu_item(pd)
                : to_next_menu_item(pd);

            n--;

            item = pd->items[pd->which];

            if ((item->flags & MENU_DISABLED) == 0)
            {
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

static void menu_left_rt(int dir)
{
    menu_bar_t *bar = active_screen->menu_bar;
    pulldown_t *pd;
    int16_t n;

    if ((bar != NULL) && (bar->active != 0))
    {
        n = bar->count;

        pd = bar->items[bar->which];
        uC_win_close(pd->window);
        pd->window = NULL;

        while (n != 0)
        {
            (dir > 0)
                ? prev_pd(bar)
                : next_pd(bar);

            n--;

            pd = bar->items[bar->which];

            if ((pd->flags & MENU_DISABLED) == 0)
            {
                redraw_pulldown(bar);
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

static void menu_cr(void)
{
    menu_bar_t *bar = active_screen->menu_bar;
    pulldown_t *pd;
    menu_item_t *item;

    if ((bar != NULL) && (bar->active != 0))
    {
        uC_set_key(-1);

        bar->active = 0;
        pd = bar->items[bar->which];

        uC_win_close(pd->window);
        pd->window = NULL;

        item = pd->items[pd->which];

        if (item->fp != NULL)
        {
            (item->fp)();
        }
    }
    else // make inactive menus not eat the new line char
    {
        uC_set_key(0x0a);
    }
}

// -----------------------------------------------------------------------

void menu_up(void)    { menu_up_down(MENU_UP);    }
void menu_down(void)  { menu_up_down(MENU_DOWN);  }
void menu_left(void)  { menu_left_rt(MENU_LEFT);  }
void menu_right(void) { menu_left_rt(MENU_RIGHT); }

// -----------------------------------------------------------------------
// you should not modify these pointers... but i can! (tm)

API void uC_menu_init(void)
{
    init_key_handlers();

    uC_set_key_action(K_ENT,  menu_cr);
    uC_set_key_action(K_CUU1, menu_up);
    uC_set_key_action(K_CUD1, menu_down);
    uC_set_key_action(K_CUB1, menu_left);
    uC_set_key_action(K_CUF1, menu_right);
    uC_set_key_action(K_F10,  menu_activate);

    // without this the cursor up and down keys can return escape
    // sequences that do not match what is specified for them in
    // the terminfo file - emitting an smkx escape sequence to the
    // terminal turns on keyboard transmit mode what ever that is

    ti_smkx();
}

// =======================================================================

