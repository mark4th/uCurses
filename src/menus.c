// uCurses menus
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "h/list.h"
#include "h/uCurses.h"

extern uint8_t attrs[8];

// -----------------------------------------------------------------------
// user application allocatted

char *status_line;

// -----------------------------------------------------------------------
// hard coded attributes for now

#define NORMAL 0x0004030000000080
#define SELECTED 0x0001060000000080
#define DISABLED 0x00080400000000c2

// -----------------------------------------------------------------------
// print string into status bar.  truncates string to fit

void bar_set_status(char *s)
{
    int16_t i;
    if(status_line != NULL)
    {
        memset(status_line, 0x20, MAX_STATUS);

        for(i = 0; i < MAX_STATUS; i++)
        {
            status_line[i] = *s++;
        }
    }
}

// -----------------------------------------------------------------------

void bar_clr_status(void)
{
    if(status_line != NULL)
    {
        memset(status_line, 0x20, MAX_STATUS);
    }
}

// -----------------------------------------------------------------------

void alloc_status(void)
{
    status_line = calloc(MAX_STATUS + 1, 1);

    if(status_line != NULL)
    {
        bar_clr_status();
    }
}

// -----------------------------------------------------------------------
// i knew i forgot something!

static void free_status(void) { free(status_line); }

// -----------------------------------------------------------------------

void bar_draw_status(menu_bar_t *bar)
{
    uint16_t x;
    screen_t *scr;

    if((bar != NULL) && (status_line != NULL))
    {
        window_t *win = bar->window;
        scr = win->screen;
        x = (scr->width - MAX_STATUS) - 6;
        win_cup(win, x, 0);
        win_emit(win, '[');
        win_emit(win, ' ');
        win_puts(win, status_line);
        win_emit(win, ']');
    }
}

// -----------------------------------------------------------------------

static INLINE uint32_t init_bar(screen_t *scr, window_t *win,
                                menu_bar_t *bar)
{
    bar->window = win;

    // make window non scrolling
    win->flags = WIN_LOCKED;

    win->screen = scr;
    scr->menu_bar = bar;

    *(uint64_t *)bar->normal = NORMAL;
    *(uint64_t *)bar->selected = SELECTED;
    *(uint64_t *)bar->disabled = DISABLED;

    bar->xco = 2; // x coordinate of first pulldown menu window
    return 0;
}

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
            return init_bar(scr, win, bar);
        }
        free(bar);
        free(win);
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

    if((bar != NULL) && (bar->count != MAX_MENU_ITEMS))
    {
        pd = calloc(1, sizeof(pulldown_t));

        if(pd != NULL)
        {
            bar->items[bar->count++] = pd;

            pd->name = name;
            pd->xco = bar->xco;

            bar->xco += utf8_strlen(name) + 2;

            *(uint64_t *)pd->normal = NORMAL;
            *(uint64_t *)pd->selected = SELECTED;
            *(uint64_t *)pd->disabled = DISABLED;

            result = 0;
        }
    }

    return result;
}

// -----------------------------------------------------------------------

static INLINE uint32_t init_item(pulldown_t *pd, menu_item_t *item,
                                 char *name, menu_fp_t fp,
                                 uint16_t shortcut)
{
    uint16_t width;

    pd->items[pd->count++] = item;

    item->name = name;
    item->fp = fp;
    item->shortcut = shortcut;

    // keep track of which item in menu is widest as that will
    // determine the width of the pulldown window

    width = utf8_strlen(name);

    if(width > pd->width)
    {
        pd->width = width;
    }

    return 0;
}

// -----------------------------------------------------------------------

static INLINE uint32_t new_item(pulldown_t *pd, char *name, menu_fp_t fp,
                                uint16_t shortcut)
{
    uint32_t result = -1;

    if(pd->count != MAX_MENU_ITEMS)
    {
        menu_item_t *item = calloc(1, sizeof(menu_item_t));

        if(item != NULL)
        {
            result = init_item(pd, item, name, fp, shortcut);
        }
    }
    return result;
}

// -----------------------------------------------------------------------
// add an entry to a pulldown

uint32_t new_menu_item(screen_t *scr, char *name, menu_fp_t fp,
                       uint16_t shortcut)
{
    uint32_t result = -1; // assume failure

    pulldown_t *pd;
    menu_bar_t *bar;

    if((scr != NULL) && (scr->menu_bar != NULL))
    {
        bar = scr->menu_bar;
        pd = bar->items[bar->count - 1];

        if(pd != NULL)
        {
            result = new_item(pd, name, fp, shortcut);
        }
    }

    return result;
}

// -----------------------------------------------------------------------
// populate pulldown window with menu items

void bar_populdate_pd(pulldown_t *pd)
{
    menu_item_t *item;
    window_t *win;
    uint16_t i;

    if((pd != NULL) && (pd->count != 0) && (pd->window != NULL))
    {
        win = pd->window;
        win_clear(win);

        for(i = 0; i != pd->count; i++)
        {
            item = pd->items[i];

            *(uint64_t *)win->attrs =
                (i == pd->which)                ? *(uint64_t *)pd->selected
                : (item->flags & MENU_DISABLED) ? *(uint64_t *)pd->disabled
                                                : *(uint64_t *)pd->normal;

            win_cup(win, 0, i);
            win_puts(win, item->name);

            while((win->cx != win->width) && (win->cx != 0))
            {
                win_emit(win, win->blank);
            }
        }
    }
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

        *(uint64_t *)win->attrs = *(uint64_t *)bar->normal;

        win_clear(win);
        win_emit(win, win->blank);

        for(i = 0; i < bar->count; i++)
        {
            pd = bar->items[i];

            *(uint64_t *)win->attrs =
                ((i == bar->which) && (bar->active != 0))
                    ? *(uint64_t *)bar->selected
                : (pd->flags & MENU_DISABLED) ? *(uint64_t *)bar->disabled
                                              : *(uint64_t *)bar->normal;

            win_emit(win, win->blank);
            win_puts(win, pd->name);
            win_emit(win, win->blank);
        }

        *(uint64_t *)win->attrs = *(uint64_t *)bar->normal;

        while((win->cx != win->width) && (win->cx != 0))
        {
            win_emit(win, win->blank);
        }
    }
}

// -----------------------------------------------------------------------

static INLINE void pd_close(pulldown_t *pd)
{
    uint16_t i;

    if(pd != NULL)
    {
        for(i = 0; i != pd->count; i++)
        {
            free(pd->items[i]);
        }

        // window only exists if this pulldown is currently active
        win_close(pd->window);
        free(pd);
    }
}

// -----------------------------------------------------------------------
// cloase all pulldowns atached to the bar

static INLINE void bar_close_pds(menu_bar_t *bar)
{
    uint16_t i;
    pulldown_t *pd;

    for(i = 0; i != bar->count; i++)
    {
        pd = bar->items[i];
        pd_close(pd);
    }
}

// -----------------------------------------------------------------------

void bar_close(screen_t *scr)
{
    menu_bar_t *bar;

    if(scr != NULL)
    {
        bar = scr->menu_bar;

        if(bar != NULL)
        {
            bar_close_pds(bar);
            win_close(bar->window);
            free_status();
            free(bar);
        }
    }
}

// -----------------------------------------------------------------------
// find address of pulldown structure with specified name

static pulldown_t *pd_find(screen_t *scr, char *name)
{
    pulldown_t *pd = NULL;
    menu_bar_t *bar = scr->menu_bar;
    uint32_t name_hash = fnv_hash(name);

    uint16_t i;

    for(i = 0; i < bar->count; i++)
    {
        pd = bar->items[i];

        if(pd != NULL)
        {
            if(fnv_hash(pd->name) == name_hash)
            {
                break;
            }
            // affects a continue and leaves pd = NULL
            // if we reached the end of the loop
            pd = NULL;
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
// create a new window for the pulldown menu we are about to display

static INLINE uint32_t bar_create_pd_win(screen_t *scr, pulldown_t *pd)
{
    window_t *win;
    uint32_t result = 1;

    win = win_open(pd->width, pd->count);

    if(win != NULL)
    {
        win->xco = pd->xco;
        win->yco = 2;

        win->flags = WIN_BOXED | WIN_LOCKED;
        win->blank = 0x20;

        win->bdr_attrs[ATTR] = FG_GRAY | BG_GRAY | BOLD;
        win->bdr_attrs[FG] = 11; // pd->attr[FG];
        win->bdr_attrs[BG] = 4;  // pd->attr[BG];
        win->bdr_type = BDR_CURVED;

        win->screen = scr;
        pd->window = win;

        result = 0;
    }

    return result;
}

// -----------------------------------------------------------------------

static void redraw_pulldown(menu_bar_t *bar)
{
    pulldown_t *pd = bar->items[bar->which];

    if(pd->count != 0)
    {
        if(bar_create_pd_win(active_screen, pd) != 0)
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

    if(bar->active != 0)
    {
        redraw_pulldown(bar);
    }
    else
    {
        win_close(pd->window);
        pd->window = NULL;
    }
}

// -----------------------------------------------------------------------

static INLINE void prev_item(pulldown_t *pd)
{
    pd->which = (pd->which != 0) ? pd->which - 1 : pd->count - 1;
}

// -----------------------------------------------------------------------

static INLINE void next_item(pulldown_t *pd)
{
    pd->which = (pd->which != pd->count - 1) ? pd->which + 1 : 0;
}

// -----------------------------------------------------------------------

static INLINE void next_pd(menu_bar_t *bar)
{
    bar->which = (bar->which != bar->count - 1) ? bar->which + 1 : 0;
}

// -----------------------------------------------------------------------

static INLINE void prev_pd(menu_bar_t *bar)
{
    bar->which = (bar->which != 0) ? bar->which - 1 : bar->count - 1;
}

// -----------------------------------------------------------------------

static void menu_up(void)
{
    menu_bar_t *bar = active_screen->menu_bar;
    pulldown_t *pd;
    uint16_t n;
    menu_item_t *item;

    if((bar != NULL) && (bar->active != 0))
    {
        pd = bar->items[bar->which];
        n = bar->count;

        while(n != 0)
        {
            prev_item(pd);
            n--;
            item = pd->items[pd->which];
            if((item->flags & MENU_DISABLED) == 0)
            {
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

static void menu_down(void)
{
    menu_bar_t *bar = active_screen->menu_bar;
    pulldown_t *pd;
    uint16_t n;
    menu_item_t *item;

    if((bar != NULL) && (bar->active != 0))
    {
        pd = bar->items[bar->which];
        n = bar->count;

        while(n != 0)
        {
            next_item(pd);
            n--;
            item = pd->items[pd->which];

            if((item->flags & MENU_DISABLED) == 0)
            {
                break;
            }
        }
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

        pd = bar->items[bar->which];
        win_close(pd->window);
        pd->window = NULL;

        while(n != 0)
        {
            prev_pd(bar);
            n--;
            pd = bar->items[bar->which];

            if((pd->flags & MENU_DISABLED) == 0)
            {
                redraw_pulldown(bar);
                break;
            }
        }
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

        pd = bar->items[bar->which];
        win_close(pd->window);
        pd->window = NULL;

        while(n != 0)
        {
            next_pd(bar);
            n--;
            pd = bar->items[bar->which];

            if((pd->flags & MENU_DISABLED) == 0)
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

    if((bar != NULL) && (bar->active != 0))
    {
        stuff_key(0xff);

        bar->active = 0;
        pd = bar->items[bar->which];

        win_close(pd->window);

        pd->window = NULL;
        item = pd->items[pd->which];
        if(item->fp != NULL)
        {
            (item->fp)();
        }
    }
    else
    {
        stuff_key(0x0a);
    }
}

// -----------------------------------------------------------------------
// you should not modify these pointers... but i can! (tm)

void menu_init(void)
{
    init_key_handlers();

    set_key_action(K_ENT, menu_cr);
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
