// screen.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h/list.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

screen_t *active_screen;

extern uint8_t attrs[8];
extern uint8_t old_attrs[8];

// -----------------------------------------------------------------------

uint16_t scr_alloc(screen_t *scr)
{
    cell_t *p1, *p2;

    // allocate buffers 1 and 2 for screen
    p1 = calloc((scr->width * scr->height), sizeof(*p1));
    p2 = calloc((scr->width * scr->height), sizeof(*p2));

    if((p1 == NULL) || (p2 == NULL))
    {
        free(p1);
        free(p2);
        return -1;
    }

    scr->buffer1 = p1;
    scr->buffer2 = p2;

    return 0;
}

// -----------------------------------------------------------------------

screen_t *scr_open(uint16_t width, uint16_t height)
{
    screen_t *scr = calloc(1, sizeof(*scr));

    if(scr != NULL)
    {
        scr->width  = width;
        scr->height = height;

        if(scr_alloc(scr) != 0)
        {
            free(scr);
            scr = NULL;
        }
    }
    return scr;
}

// -----------------------------------------------------------------------
// attach a window to a screen

void scr_win_attach(screen_t *scr, window_t *win)
{
    win->screen = scr;

    if(list_append_node(&scr->windows, win) != 0)
    {
        // log error here?
        // insert more ram to conginue!
    }
}

// -----------------------------------------------------------------------
// detach window from its parent screen

void scr_win_detach(window_t *win)
{
    screen_t *scr = win->screen;

    if(scr != NULL)
    {
        list_remove_node(&scr->windows, win);
        win->screen = NULL;
    }
}

// -----------------------------------------------------------------------

void scr_close(screen_t *scr)
{
    window_t *win;

    free(scr->buffer1);
    free(scr->buffer2);

    scr->buffer1 = NULL;
    scr->buffer2 = NULL;

    win_close(scr->backdrop);

    while((win = list_pop(&scr->windows)) != NULL)
    {
        win_close(win);
    }
    bar_close(scr);

    free(scr);
}

// -----------------------------------------------------------------------

static cell_t *scr_line_addr(screen_t *scr, uint16_t line)
{
    uint16_t index = (scr->width) * line;
    return &scr->buffer1[index];
}

// -----------------------------------------------------------------------
// draw window into its parent screen with borders if it has them

static void scr_draw_win(window_t *win)
{
    uint16_t i;
    cell_t *src, *dst;
    uint32_t q;

    if(win != NULL)
    {
        screen_t *scr = win->screen;

        dst = scr_line_addr(scr, win->yco);
        dst = &dst[win->xco];
        src = win->buffer;

        q = win->width * sizeof(cell_t);

        for(i = 0; i < win->height; i++)
        {
            memcpy(dst, src, q);
            dst += scr->width;
            src += win->width;
        }

        // draw windows border if it has one
        if(win->flags & WIN_BOXED)
        {
            win_draw_borders(win);
        }
    }
}

// -----------------------------------------------------------------------

static void scr_draw_windows(screen_t *scr)
{
    window_t *win;
    node_t *n = scr->windows.head;

    while(n != NULL)
    {
        win = n->payload;
        scr_draw_win(win);
        n = n->next;
    }
}

// -----------------------------------------------------------------------
// set terminal cursor location to same location as screens cursor

// unless it is already there :)

void scr_cup(screen_t *scr, uint16_t x, uint16_t y)
{
    if((x != scr->cx) || (y != scr->cy))
    {
        cup(y, x);
    }
    scr->cx = x;
    scr->cy = y;
}

// -----------------------------------------------------------------------

static uint16_t scr_is_modified(screen_t *scr, uint16_t index)
{
    uint16_t result;

    cell_t *p1 = &scr->buffer1[index];
    cell_t *p2 = &scr->buffer2[index];

    // if attrs of this cell in buffer1 are different from the attrs
    // in buffer 2 or if the characters in those cells are different
    // then this cell needs updating

    result = (*(uint64_t *)&p1->attrs != *(uint64_t *)p2->attrs);
    result |= (p1->code != p2->code);

    return result;
}

// -----------------------------------------------------------------------
// emits a charcter in the screen buffer1 out to the console

static void scr_emit(screen_t *scr, uint16_t index)
{
    uint16_t x, y;
    cell_t *p1, *p2;

    p1 = &scr->buffer1[index];
    p2 = &scr->buffer2[index];

    *p2 = *p1;               // mark cell as no longer needing update

    y = index / scr->width;  // convert index to coordinates
    x = index % scr->width;

    scr_cup(scr, x, y);      // hopefylly only sets x or y if not correct

    // this horrendous code ensures that any double wide character such
    // as a chinese character that is not immediately followed by another
    // double wide character is not drawn.  if it were drawn it would then
    // overwrite the single wide char that follows it and that char would
    // then not be redrawn because as far as the system is concerned
    // that cell never changed

    if(is_wide(p1->code) == 1)
    {
        if(is_wide((p1 + 1)->code) == 0)
        {
            utf8_emit(0x20);
            scr->cx++;
            (p2+1)->code = 0;
            *(uint64_t *)&(p2+1)->attrs = 0;
            goto skip;
        }
    }

    utf8_emit(p1->code);     // output utf-8 codepoint to terminal

skip:
    scr->cx++;

    if(scr->cx == scr->width)
    {
        scr->cx = 0;
        scr->cy++;
    }
}

// -----------------------------------------------------------------------
// add a backdrop window to the screen

void scr_add_backdrop(screen_t *scr)
{
    window_t *win = win_open(scr->width  - 2, scr->height - 2);

    if(win != NULL)
    {
        win->xco    = 1;
        win->yco    = 1;

        win->flags  = WIN_BOXED | WIN_LOCKED;
        win->blank  = SOLID;
        win->screen = scr;

        win_set_gray_fg(win, 12);

        win->bdr_attrs[ATTR] = FG_GRAY | BG_GRAY | BOLD;
        win->bdr_attrs[FG] = 13;
        win->bdr_attrs[BG] = 0;
        win->bdr_type = BDR_SINGLE;

        win_clear(win);

        scr->backdrop = win;
    }
}

// -----------------------------------------------------------------------
// inner loop of screen update

static uint32_t inner_update(screen_t *scr, uint16_t index,
    uint16_t end)
{
    cell_t *p1;
    int indx = 0;

    p1 = &scr->buffer1[index];

    *(uint64_t *)&attrs = *(uint64_t *)&p1->attrs;

    apply_attribs();

    p1 = &scr->buffer1[index];

    do
    {
        if(*(uint64_t *)&attrs == *(uint64_t *)&p1->attrs)
        {
            scr_emit(scr, index);
        }
        else
        {
            if(indx == 0)
            {
                indx = index;
            }
        }
        index++;  p1++;
    } while (index != end);

    // return index of first char that had different attributes to the
    // ones we were updating
    return indx;
}

// -----------------------------------------------------------------------
// outer loop of screen update

static void outer_update(screen_t *scr)
{
    uint16_t index = 0;
    uint16_t end = scr->width * scr->height;

    // force a screen cursor position update
    scr->cx = scr->cy = -1;

    do
    {
        // if char at index is modified then output everey char in the
        // screen that shares its attributes.
        if(scr_is_modified(scr, index) != 0)
        {
            index = inner_update(scr, index, end) - 1;

            // the return value is the index of the first character
            // that update found that had different atributes to the
            // ones it was setting.  this is our new scan point
            // if indx is zero then update scanned to the end of the
            // screen and found no characters that that it did not
            // already update so we can exit early too

            if(index == 0) { break; }
            continue;  // skip the ++
        }
        index++;
    } while(index != end);
}

// -----------------------------------------------------------------------

static void scr_update_menus(screen_t *scr)
{
    menu_bar_t *bar;
    pulldown_t *pd;

    if(scr->menu_bar != NULL)
    {
        bar = scr->menu_bar;

        // draw all text into memu bar window then write that to
        // the screen buffer
        bar_draw_text(scr);
        scr_draw_win(bar->window);

        if(bar->active != 0)
        {
            pd = bar->items[bar->which];

            // draw all text inside pulldown memus window
            bar_populdate_pd(pd);
            // draw pulldown window into screen
            scr_draw_win((window_t *)pd->window);
        }
    }
}

// -----------------------------------------------------------------------

void scr_draw_screen(screen_t *scr)
{
    active_screen = scr;

    *(uint64_t *)&old_attrs[0] = 0;

    // backdrop if it exists is always the first window to be drawn into
    // the screen
    scr_draw_win((window_t *)scr->backdrop);

    scr_draw_windows(scr);
    scr_update_menus(scr);

    outer_update(scr);

    flush();
}

// =======================================================================
