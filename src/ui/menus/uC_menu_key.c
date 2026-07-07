// menu_key.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

#include "uC_menus.h"
#include "uC_keys.h"
#include "uC_terminfo.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

#ifdef UC_MENUS

// -----------------------------------------------------------------------

static uC_screen_t *menu_screen;
static bool menu_keys_initialized;
static uC_key_handler_t *menu_saved_default_f10;
static uC_key_handler_t *menu_saved_current_f10;

void menu_set_screen(uC_screen_t *scr)
{
    menu_screen = scr;
}

// -----------------------------------------------------------------------

static bool pd_enabled(pulldown_t *pd)
{
    return (pd != NULL) && ((pd->flags & uC_MENU_DISABLED) == 0);
}

// -----------------------------------------------------------------------

static bool item_enabled(menu_item_t *item)
{
    return (item != NULL) && ((item->flags & uC_MENU_DISABLED) == 0);
}

// -----------------------------------------------------------------------

static menu_item_t *selected_item(pulldown_t *pd)
{
    if (pd == NULL)
    {
        return NULL;
    }

    return pd->items[pd->which];
}

// -----------------------------------------------------------------------

static uint8_t menu_key_return(uint8_t key)
{
    if (key == 0xff)
    {
        return 0;
    }

    return key;
}

// -----------------------------------------------------------------------

static uC_key_handler_t *saved_f10_or_noop(uC_key_handler_t *saved)
{
    if (saved != NULL)
    {
        return saved;
    }

    return uC_noop;
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
    if (pd->which != pd->count - 1)
    {
        pd->which++;
        return;
    }

    pd->which = 0;
}

// -----------------------------------------------------------------------

static void next_pd(menu_bar_t *bar)
{
    if (bar->which != bar->count - 1)
    {
        bar->which++;
        return;
    }

    bar->which = 0;
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

static bool select_enabled_pd(menu_bar_t *bar, int dir, bool advance)
{
    int16_t n;

    if ((bar == NULL) || (bar->count == 0))
    {
        return false;
    }

    if ((bar->which < 0) || (bar->which >= bar->count))
    {
        bar->which = 0;
    }

    for (n = 0; n < bar->count; n++)
    {
        if (advance || (n != 0))
        {
            if (dir > 0)
            {
                prev_pd(bar);
            }
            else
            {
                next_pd(bar);
            }
            advance = false;
        }

        if (pd_enabled(bar->items[bar->which]))
        {
            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------

static bool select_enabled_item(pulldown_t *pd, int dir, bool advance)
{
    int16_t n;

    if ((pd == NULL) || (pd->count == 0))
    {
        return false;
    }

    if ((pd->which < 0) || (pd->which >= pd->count))
    {
        pd->which = 0;
    }

    for (n = 0; n < pd->count; n++)
    {
        if (advance || (n != 0))
        {
            if (dir > 0)
            {
                to_prev_menu_item(pd);
            }
            else
            {
                to_next_menu_item(pd);
            }
            advance = false;
        }

        if (item_enabled(pd->items[pd->which]))
        {
            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------

static void redraw_pulldown(menu_bar_t *bar)
{
    int32_t f;

    pulldown_t *pd = bar->items[bar->which];

    if (pd_enabled(pd) && (pd->count != 0))
    {
        select_enabled_item(pd, MENU_DOWN, false);
        f = bar_create_pd_win(menu_screen, pd);

        if (f != 0)
        {
            draw_pd(pd);
        }
    }
}

// -----------------------------------------------------------------------

static void menu_activate(void)
{
    menu_bar_t *bar = menu_screen->menu_bar;
    pulldown_t *pd;
    bool pd_selected;

    if (bar == NULL)
    {
        return;
    }

    if (bar->active == 0)
    {
        pd_selected = select_enabled_pd(bar, MENU_RIGHT, false);
        if (!pd_selected)
        {
            return;
        }

        bar->active = 1;
        redraw_pulldown(bar);
        return;
    }

    bar->active = 0;
    pd = bar->items[bar->which];

    if (pd != NULL)
    {
        uC_win_close(pd->window);
        pd->window = NULL;
    }
}

// -----------------------------------------------------------------------

API bool uC_menu_is_active(uC_screen_t *scr)
{
    menu_bar_t *bar = NULL;

    if (scr != NULL)
    {
        bar = scr->menu_bar;
    }

    return (bar != NULL) && (bar->active != 0);
}

// -----------------------------------------------------------------------

API void uC_menu_open(uC_screen_t *scr)
{
    menu_bar_t *bar;
    pulldown_t *pd;
    bool pd_selected;

    if ((scr == NULL) || (scr->menu_bar == NULL))
    {
        return;
    }

    menu_set_screen(scr);
    uC_menu_init_keys();

    bar = scr->menu_bar;
    if ((bar->count == 0) || (bar->active != 0))
    {
        return;
    }

    pd_selected = select_enabled_pd(bar, MENU_RIGHT, false);
    if (!pd_selected)
    {
        return;
    }

    pd = bar->items[bar->which];
    if (pd != NULL)
    {
        select_enabled_item(pd, MENU_DOWN, false);
    }
    bar->active = 1;
    redraw_pulldown(bar);
}

// -----------------------------------------------------------------------

API void uC_menu_close(uC_screen_t *scr)
{
    menu_bar_t *bar;
    pulldown_t *pd;

    if ((scr == NULL) || (scr->menu_bar == NULL))
    {
        return;
    }

    bar = scr->menu_bar;
    if (bar->active == 0)
    {
        return;
    }

    bar->active = 0;
    pd = bar->items[bar->which];
    if (pd != NULL)
    {
        uC_win_close(pd->window);
        pd->window = NULL;
    }
}

// -----------------------------------------------------------------------

void menu_normalize_selection(uC_screen_t *scr)
{
    menu_bar_t *bar;
    pulldown_t *old_pd = NULL;
    pulldown_t *pd;
    bool pd_selected;

    if ((scr == NULL) || (scr->menu_bar == NULL))
    {
        return;
    }

    menu_set_screen(scr);
    bar = scr->menu_bar;

    if ((bar->which >= 0) && (bar->which < bar->count))
    {
        old_pd = bar->items[bar->which];
    }

    pd_selected = select_enabled_pd(bar, MENU_RIGHT, false);
    if (!pd_selected)
    {
        if (old_pd != NULL)
        {
            uC_win_close(old_pd->window);
            old_pd->window = NULL;
        }
        bar->active = 0;
        return;
    }

    pd = bar->items[bar->which];
    select_enabled_item(pd, MENU_DOWN, false);

    if (bar->active != 0)
    {
        if ((old_pd != NULL) && (old_pd != pd))
        {
            uC_win_close(old_pd->window);
            old_pd->window = NULL;
        }

        if ((pd != NULL) && (pd->window == NULL))
        {
            redraw_pulldown(bar);
        }
    }
}

// -----------------------------------------------------------------------

static void menu_up_down(int dir)
{
    menu_bar_t *bar = menu_screen->menu_bar;
    pulldown_t *pd;

    if ((bar != NULL) && (bar->active != 0))
    {
        pd = bar->items[bar->which];
        select_enabled_item(pd, dir, true);
    }
}

// -----------------------------------------------------------------------

static void menu_left_rt(int dir)
{
    menu_bar_t *bar = menu_screen->menu_bar;
    pulldown_t *pd;
    bool pd_selected;

    if ((bar != NULL) && (bar->active != 0))
    {
        pd = bar->items[bar->which];

        if (!pd_enabled(pd))
        {
            pd = NULL;
        }
        if (pd != NULL)
        {
            uC_win_close(pd->window);
            pd->window = NULL;
        }

        pd_selected = select_enabled_pd(bar, dir, true);
        if (pd_selected)
        {
            pd = bar->items[bar->which];
            select_enabled_item(pd, MENU_DOWN, false);
            redraw_pulldown(bar);
        }
        else
        {
            bar->active = 0;
        }
    }
}

// -----------------------------------------------------------------------

static void menu_cr(void)
{
    menu_bar_t *bar = menu_screen->menu_bar;
    pulldown_t *pd;
    menu_item_t *item;
    bool pd_live;
    bool item_live;

    if ((bar != NULL) && (bar->active != 0))
    {
        pd = bar->items[bar->which];
        item = selected_item(pd);

        uC_set_key(-1);

        pd_live = pd_enabled(pd);
        item_live = item_enabled(item);
        if (!pd_live || !item_live)
        {
            return;
        }

        if (item->fp != NULL)
        {
            bar->active = 0;
            uC_win_close(pd->window);
            pd->window = NULL;
            (item->fp)();
        }
    }
    else // make inactive menus not eat the new line char
    {
        uC_set_key(0x0a);
    }
}

// -----------------------------------------------------------------------

static uint8_t menu_action_key(void)
{
    uint8_t key = uC_key_raw();

    return menu_key_return(key);
}

// -----------------------------------------------------------------------

static void menu_up(void)    { menu_up_down(MENU_UP);    }
static void menu_down(void)  { menu_up_down(MENU_DOWN);  }
static void menu_left(void)  { menu_left_rt(MENU_LEFT);  }
static void menu_right(void) { menu_left_rt(MENU_RIGHT); }

// -----------------------------------------------------------------------

static void menu_key_f10(void)
{
    uC_set_key(UC_KEY_F10);
}

// -----------------------------------------------------------------------

API void uC_menu_cursor_up(uC_screen_t *scr)
{
    menu_set_screen(scr);
    menu_up();
}

// -----------------------------------------------------------------------

API void uC_menu_cursor_down(uC_screen_t *scr)
{
    menu_set_screen(scr);
    menu_down();
}

// -----------------------------------------------------------------------

API void uC_menu_cursor_left(uC_screen_t *scr)
{
    menu_set_screen(scr);
    menu_left();
}

// -----------------------------------------------------------------------

API void uC_menu_cursor_right(uC_screen_t *scr)
{
    menu_set_screen(scr);
    menu_right();
}

// -----------------------------------------------------------------------

API void uC_menu_select(uC_screen_t *scr)
{
    menu_set_screen(scr);
    menu_cr();
}

// -----------------------------------------------------------------------

bool menu_key(uC_screen_t *scr, uint8_t key, uint8_t *out)
{
    menu_bar_t *bar;

    if (out != NULL)
    {
        *out = 0;
    }

    if ((scr == NULL) || (scr->menu_bar == NULL))
    {
        return false;
    }

    menu_set_screen(scr);
    uC_menu_init_keys();
    bar = scr->menu_bar;

    if (key == UC_KEY_F10)
    {
        menu_activate();
        return true;
    }

    if (bar->active == 0)
    {
        return false;
    }

    switch (key)
    {
        case UC_KEY_ESC:   uC_menu_close(scr);  return true;
        case UC_KEY_UP:    menu_up();           return true;
        case UC_KEY_DOWN:  menu_down();         return true;
        case UC_KEY_LEFT:  menu_left();         return true;
        case UC_KEY_RIGHT: menu_right();        return true;

        case '\r':
        case UC_KEY_ENTER:
            menu_cr();
            if (out != NULL)
            {
                *out = menu_action_key();
            }
            return true;

        default:
            break;
    }

    return false;
}

// -----------------------------------------------------------------------

API uint8_t uC_menu_run(uC_screen_t *scr)
{
    uint8_t key;

    if ((scr == NULL) || (scr->menu_bar == NULL))
    {
        return 0;
    }

    menu_set_screen(scr);
    uC_menu_init_keys();

    while (uC_menu_is_active(scr))
    {
        key = uC_key();
        if (key != 0)
        {
            return menu_key_return(key);
        }
        uC_scr_draw_screen(scr);
    }

    return 0;
}

// -----------------------------------------------------------------------
// you should not modify these pointers... but i can! (tm)

API void uC_menu_init_keys(void)
{
    // without this the cursor up and down keys can return escape
    // sequences that do not match what is specified for them in
    // the terminfo file - emitting an smkx escape sequence to the
    // terminal turns on keyboard transmit mode what ever that is

    ti_smkx();

    if (!menu_keys_initialized)
    {
        menu_saved_default_f10 = uC_set_default_key_action(K_F10,
            menu_key_f10);
        menu_saved_current_f10 = uC_set_key_action(K_F10, menu_key_f10);
        menu_keys_initialized = true;
    }
}

// -----------------------------------------------------------------------

void uC_menu_deinit_keys(void)
{
    if (menu_keys_initialized)
    {
        uC_restore_default_key_action(K_F10, menu_key_f10,
            saved_f10_or_noop(menu_saved_default_f10));
        uC_restore_key_action(K_F10, menu_key_f10,
            saved_f10_or_noop(menu_saved_current_f10));

        menu_saved_default_f10 = NULL;
        menu_saved_current_f10 = NULL;
        menu_keys_initialized = false;
    }
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
