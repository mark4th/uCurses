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
extern uint16_t delay_flush;

// -----------------------------------------------------------------------

static uint16_t scr_alloc(screen_t *scr)
{
    cell_t *p1, *p2;

    // allocate buffers 1 and 2 for screen
    p1 = calloc((scr->width * scr->height), sizeof(*p1));
    p2 = calloc((scr->width * scr->height), sizeof(*p2));

    if((NULL == p1) || (NULL == p2))
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

    if(NULL != scr)
    {
        scr->width  = width;
        scr->height = height;

        if(0 != scr_alloc(scr))
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
    if(0 != list_append_node(&scr->windows, win))
    {
        // log error here?
    }
}

// -----------------------------------------------------------------------
// detach window from its parent screen

void scr_win_detach(window_t *win)
{
    screen_t *scr = win->screen;

    if(NULL != scr)
    {
        list_remove_node(&scr->windows, win);
        win->screen = NULL;
    }
}

// -----------------------------------------------------------------------

void scr_close(screen_t *scr)
{
    if(0 != scr->buffer1)
    {
        free(scr->buffer1);
        scr->buffer1 = 0;
    }
    if(0 != scr->buffer2)
    {
        free(scr->buffer2);
        scr->buffer2 = 0;
    }
    if(0 != scr->backdrop)
    {
        win_close(scr->backdrop);
        free(scr->backdrop);
    }
    while(0 != scr->windows.count)
    {
         window_t *win = list_pop(&scr->windows);
         win_close(win);
    }
    free(scr);
}

// -----------------------------------------------------------------------

static void scr_draw_windows(screen_t *scr)
{
    node_t *n = scr->windows.head;

    while(NULL != n)
    {
       win_draw(n->payload);
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

    // if attrs of this cell in buffer1 are different from the addrs
    // in buffer 2 or if the characters in those cells are different
    // then this cell needs updating
    return(
        (memcmp(&p1->attrs, &p2->attrs, 8)) ||
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

static uint32_t update(screen_t *scr, uint16_t index, uint16_t end)
{
    cell_t *p1;
    int indx = 0;

    p1 = &scr->buffer1[index];

    memcpy(&attrs[0], &p1->attrs[0], 8);
    apply_attribs();

    p1 = &scr->buffer1[index];
    do
    {
        if(0 == memcmp(&attrs[0], &p1->attrs, 8))
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

//    delay_flush = -1;

    memset(&old_attrs[0], 0, 8);

    win_draw((window_t *)scr->backdrop);
    scr_draw_windows(scr);

    scr->cx = scr->cy = -1;

    do
    {
        // if char at index is modified then output everey char in the
        // screen that shares its attributes.
        if(0 != scr_is_modified(scr, index))
        {
            indx = update(scr, index, end);
            if(indx != 0) { index = indx - 1; }
        }
        index++;
    } while(index != end);

//    delay_flush = 0;
//    flush();
}

// -----------------------------------------------------------------------
// add a backdrop window to the screen

void scr_add_backdrop(screen_t *scr)
{
    window_t *win = win_open(scr->width  - 2, scr->height - 2);

    if(NULL != win)
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

// =======================================================================
