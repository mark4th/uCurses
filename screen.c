// screen.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h/list.h"
#include "h/tui.h"
#include "h/uCurses.h"
#include "h/util.h"

// -----------------------------------------------------------------------

screen_t *current_screen;

extern uint8_t attrs[8];
extern uint8_t old_attrs[8];

// -----------------------------------------------------------------------

static uint16_t scr_alloc(screen_t *scr)
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
    scr->buffer1 = 0;
    scr->buffer2 = 0;
    win_close(scr->backdrop);
    free(scr->backdrop);

    while(scr->windows.count != 0)
    {
         win = list_pop(&scr->windows);
         win_close(win);
    }
    free(scr);
}

// -----------------------------------------------------------------------
// draw window into its parent screen with borders if it has them

static void scr_draw_win(window_t *win)
{
    uint16_t i;
    cell_t *src, *dst;

    screen_t *scr = win->screen;

    // borders must be drawn first
    if(win->flags & WIN_BOXED)
    {
        win_draw_borders(win);
    }

    dst = &scr->buffer1[(win->yco * scr->width) + win->xco];
    src = win->buffer;

    for(i = 0; i < win->height; i++)
    {
        memcpy(dst, src, win->width * sizeof(cell_t));
        dst += scr->width;
        src += win->width;
    }
}

// -----------------------------------------------------------------------

static void scr_draw_windows(screen_t *scr)
{
    node_t *n = scr->windows.head;

    while(n != NULL)
    {
       scr_draw_win(n->payload);
       n = n->next;
    }
}

// -----------------------------------------------------------------------
// set terminal cursor location to same location as screens cursor

// unless it is already there :)

void scr_cup(screen_t *scr, uint16_t x, uint16_t y)
{
    if((x != scr->cx) && (y != scr->cy))
    {
        cup(y, x);
    }
    else if(x != scr->cx)
    {
        hpa(x);
    }
    else if(y != scr->cy)
    {
        vpa(y);
    }
    scr->cx = x;
    scr->cy = y;
}

// -----------------------------------------------------------------------

static uint16_t scr_is_modified(screen_t *scr, uint16_t index)
{
    cell_t *p1 = &scr->buffer1[index];
    cell_t *p2 = &scr->buffer2[index];

    // if attrs of this cell in buffer1 are different from the attrs
    // in buffer 2 or if the characters in those cells are different
    // then this cell needs updating

    return(
        *(uint64_t *)&p1->attrs != *(uint64_t *)p2->attrs ||
        (p1->code != p2->code));
}

// -----------------------------------------------------------------------

static void scr_emit(screen_t *scr, uint16_t index)
{
    uint16_t x, y;
    cell_t *p1, *p2;

    p1 = &scr->buffer1[index];
    p2 = &scr->buffer2[index];

    *p2 = *p1;               // mark cell as no longer needing update

    // convert index to coordinates
    y = index / scr->width;
    x = index % scr->width;

    // this only sets x or y if they are not already correct
    scr_cup(scr, x, y);

    // output the utf-8 codepoint to the terminal
    utf8_emit(p1->code);

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
        win->bdr_type = BDR_DOUBLE;
        win_clear(win);

        scr->backdrop = win;
    }
}

// -----------------------------------------------------------------------

static uint32_t update(screen_t *scr, uint16_t index, uint16_t end)
{
    cell_t *p1;
    int indx = 0;

    p1 = &scr->buffer1[index];

    *(uint64_t *)&attrs[0] = *(uint64_t *)&p1->attrs[0];
    apply_attribs();

    p1 = &scr->buffer1[index];
    do
    {
        if(*(uint64_t *)&attrs[0] == *(uint64_t *)&p1->attrs)
        {
            scr_emit(scr, index);
        }
        else
        {
            if(0 == indx)
            {
                indx = index;
            }
        }
        index++;
        p1++;
    } while (index != end);

    return indx;
}

// -----------------------------------------------------------------------

void scr_do_draw_screen(screen_t *scr)
{
    uint16_t index = 0, indx;
    uint16_t end = scr->width * scr->height;

    memset(&old_attrs[0], 0, 8);

    scr_draw_win((window_t *)scr->backdrop);
    scr_draw_windows(scr);

    scr->cx = scr->cy = -1;

    do
    {
        // if char at index is modified then output everey char in the
        // screen that shares its attributes.
        if(scr_is_modified(scr, index) != 0)
        {
            indx = update(scr, index, end);
            if(indx != 0)
            {
                index = indx;
                continue;
            }
        }
        index++;
    } while(index != end);

    flush();
}

// =======================================================================
