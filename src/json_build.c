// json_build.c   - finish construction of json defined interface
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

extern uint16_t console_width;
extern uint16_t console_height;

// -----------------------------------------------------------------------
// vereify window is within bounds of screen

static void bounds_check(window_t *win)
{
    uint16_t xco = win->xco;
    uint16_t yco = win->yco;

    uint16_t z = (win->flags & WIN_BOXED) ? 1 :0;

    if(((xco + win->width  + z) < console_width)  &&
       ((yco + win->height + z) < console_height) &&
       ((xco - z) < console_width) &&
       ((yco - z) < console_height))
    {
        return;
    }

    json_error("Window outside bounds of screen");
}

// -----------------------------------------------------------------------
// fix far window position

static void fix_win(screen_t *scr, window_t *win)
{
    uint16_t fudge = 1;
    window_t *bd = scr->backdrop;

    if((bd != NULL) && ((bd->flags & WIN_BOXED) != 0))
    {
        fudge++;
    }

    if(win->xco == FAR)
    {
        win->xco = scr->width - (win->width + fudge);
    }

    if(win->yco == FAR)
    {
        win->yco = scr->height - (win->height + fudge);
    }
}

// -----------------------------------------------------------------------
// complete init of windows now we know width/height etc

static void fix_windows(screen_t *scr)
{
    window_t *win;
    if(scr->backdrop != NULL)
    {
        init_backdrop(scr, scr->backdrop);
        win_alloc(scr->backdrop);
        win_clear(scr->backdrop);
    }

    node_t *n = scr->windows.head;

    while(n != NULL)
    {
        win = n->payload;
        fix_win(win->screen, win);
        bounds_check(win);
        win_alloc(win);
        win->blank = 0x20;
        win_clear(win);
        n = n->next;
    }
}

// -----------------------------------------------------------------------

static void fix_menus(screen_t *scr)
{
    menu_bar_t *bar = scr->menu_bar;
    pulldown_t *pd;
    window_t *win;

    uint16_t i, j;
    uint16_t width;

    win = win_open(scr->width, 1);
    if(win == NULL)
    {
        json_error("Unable to create window for menu bar");
    }

    bar->window = win;
    win->flags = WIN_LOCKED;
    win->screen   = scr;
    scr->menu_bar = bar;
    bar->xco = 2;

    for(i = 0; i < bar->count; i++)
    {
        pd = bar->items[i];
        // set the x coordinate where the pull down window will be
        // drawn when activated
        pd->xco = bar->xco;
        bar->xco += strlen(pd->name) + 2;

        // widest menu item in pulldown menu defines the width of
        // the window
        for(j = 0; j < pd->count; j++)
        {
            width = strlen(pd->items[j]->name);
            if(pd->width <width)
            {
                pd->width = width;
            }
        }
    }
}

// -----------------------------------------------------------------------

void json_build_ui(void)
{
    uint16_t result;
    screen_t *scr = active_screen;

    result = scr_alloc(scr);
    if(result != 0)
    {
        json_error("Out of memory allocating screen");
    }

    fix_windows(scr);
    fix_menus(scr);
}

// =======================================================================
