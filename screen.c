// screen.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdbool.h>
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

// -----------------------------------------------------------------------

bool scr_alloc(screen_t *scr)
{
    void *p1;
    void *p2;

    uint32_t size = ((scr->width + scr->height) * sizeof(cell_t));

    // allocate buffers 1 and 2 for screen
    p1 = malloc(size);
    p2 = malloc(size);

    if((NULL == p1) || (NULL == p2))
    {
        if(p1) { free(p1); }
        if(p2) { free(p2); }

        return false;
    }

    scr->buffer1 = p1;
    scr->buffer2 = p2;

    return true;
}

// -----------------------------------------------------------------------
// attach a window to a screen

void scr_win_attach(screen_t *scr, window_t *win)
{
    win->screen = scr;
    list_append_node(scr->windows, win);
}

// -----------------------------------------------------------------------
// detach window from its parent screen

void scr_win_detach(window_t *win)
{
    screen_t *scr = win->screen;

    if(NULL != scr)
    {
        list_remove_node(scr->windows, win);
        win->screen = NULL;
    }
}

// -----------------------------------------------------------------------

bool open_screen(uint16_t width, uint16_t height, screen_t *scr)
{
    memset(scr, 0, sizeof(screen_t));
    scr->width = width;
    scr->height = height;

    return scr_alloc(scr);
}

// -----------------------------------------------------------------------

void close_screen(screen_t *scr)
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
}

// -----------------------------------------------------------------------

void scr_draw_windows(screen_t *scr)
{
    node_t *n = scr->windows->head;

    while(n)
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
        cup(x, y);
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

static bool scr_is_modified(screen_t *scr, uint16_t n)
{
    cell_t *p1 = &scr->buffer1[n];
    cell_t *p2 = &scr->buffer2[n];

    return memcmp(p1, p2, sizeof(cell_t))
        ? 0 : 1;
}

// -----------------------------------------------------------------------

static void scr_emit(screen_t *scr, uint16_t index)
{
    uint16_t x, y;

    cell_t *p1, *p2;

    p1 = &scr->buffer1[index];
    p2 = &scr->buffer2[index];

    *p2 = *p1;    // copy cell from buffer1 to buffer2

    // c is so crippled it cant return both the quotient and the
    // remainder from one single division operation so we avoid
    // doing two divisions by doing one division and one multiplication
    // instead - which is still dumn.

    y = index / scr->width;
    x = index - (y * scr->width);

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
// dry attribs: output all chars that share these attribs

static void update(screen_t *scr, uint16_t end, uint16_t index)
{
    cell_t *p1;

    p1 = &scr->buffer1[index];
    memcpy(&attrs[0], &p1->attrs[0], 8);
    set_attribs();

    scr_emit(scr, index++);

    // now output every other char on this screen
    // that have identical attribs

    do
    {
        index++;
        p1 = &scr->buffer1[index];
        if(0 == memcmp(&attrs[0], p1, 8))
        {
            scr_emit(scr, index++);
        }
    } while (index != end);
}

// -----------------------------------------------------------------------

void do_draw_screen(screen_t *scr)
{
    uint16_t index = 0;
    uint16_t end = scr->width * scr->height;

    do
    {
        if(scr_is_modified(scr, index))
        {
            update(scr, end, index);
        }
        index++;
    } while(index != end);
}

// =======================================================================
