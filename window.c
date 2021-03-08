// window.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h/uCurses.h"
#include "h/color.h"
#include "h/tui.h"

extern uint8_t default_bg;
extern uint8_t default_fg;

// -----------------------------------------------------------------------

static cell_t *win_line_addr(window_t *win, uint16_t line)
{
    uint16_t index = (win->width) * line;
    return &win->buffer[index];
}

// -----------------------------------------------------------------------
// allocate buffer for window contents

static uint16_t win_alloc(window_t *win)
{
    uint16_t rv = -1;   // assume failure
    cell_t *p;

    p = calloc((win->width * win->height), sizeof(cell_t));

    if(NULL != p)
    {
        win->buffer = p;
        rv = 0;
    }
    return rv;
}

// -----------------------------------------------------------------------

void win_close(window_t *win)
{
    if(0 != win)
    {
        if(0 != win->buffer)
        {
            free(win->buffer);
        }
        free(win);
    }
}

// -----------------------------------------------------------------------

window_t *win_open(uint16_t width, uint16_t height)
{
    window_t *win = calloc(1, sizeof(*win));

    if(0 != win)
    {
        win->height  = height;
        win->width   = width;

        // win_alloc() uses width/height to determine how much
        // space needs to be malloc'd
        if(0 == win_alloc(win))
        {
            win->attrs[FG] = default_fg;
            win->attrs[BG] = default_bg;
            win->blank   = 0x20;
            win_clear(win);
        }
        else
        {
            free(win);
            win = NULL;
        }
    }
    return win;
}

// -----------------------------------------------------------------------
// pop window to front

void win_pop(window_t *win)
{
    screen_t *scr = win->screen;
    scr_win_detach(win);
    scr_win_attach(scr, win);
}

// -----------------------------------------------------------------------
// set new x/y position of window within parent screen

// look ma!   moveable, overlapping windows with text scrolling in any of
// eight directions!!!

uint16_t win_set_pos(window_t *win, uint16_t x, uint16_t y)
{
    uint16_t scr_width, scr_height;
    uint16_t win_width, win_height;
    uint16_t win_x, win_y;
    uint16_t rv = -1;

    screen_t *scr = win->screen;

    win_width  = win->width;
    win_height = win->height;
    win_x      = x;
    win_y      = y;
    scr_width  = scr->width;
    scr_height = scr->height;

    // if window is boxed account for border
    if(win->flags & WIN_BOXED)
    {
        win_width += 2;
        win_height += 2;
        win_x--;
        win_y--;
    }

    if((win_x + win_width  < scr_width) &&
       (win_y + win_height < scr_height))
    {
        win->xco = x;
        win->yco = y;

        rv = 0;
    }

    return rv;
}

// -----------------------------------------------------------------------

static void win_set_attr(window_t *win, ti_attrib_t attr)
{
    win->attrs[ATTR] |= attr;

    // gray scales and rgb are mutually exclusive
    if(FG_RGB == attr)   { win->attrs[ATTR] &= ~FG_GRAY; }
    if(BG_RGB == attr)   { win->attrs[ATTR] &= ~BG_GRAY; }
    if(FG_GRAY == attr)  { win->attrs[ATTR] &= ~FG_RGB;  }
    if(BG_GRAY == attr)  { win->attrs[ATTR] &= ~BG_RGB;  }
}

// -----------------------------------------------------------------------

void win_set_gray_fg(window_t *win, uint8_t c)
{
    win->attrs[FG] = c;
    win_set_attr(win, FG_GRAY);
}

// -----------------------------------------------------------------------

void win_set_gray_bg(window_t *win, uint8_t c)
{
    win->attrs[BG] = c;
    win_set_attr(win, BG_GRAY);
}

// -----------------------------------------------------------------------

void win_set_rgb_fg(window_t *win, uint8_t r, uint8_t g, uint8_t b)
{
    win->attrs[FG_R] = r;
    win->attrs[FG_G] = g;
    win->attrs[FG_B] = b;

    win_set_attr(win, FG_RGB);
}

// -----------------------------------------------------------------------

void win_set_rgb_bg(window_t *win, uint8_t r, uint8_t g, uint8_t b)
{
    win->attrs[BG_R] = r;
    win->attrs[BG_G] = g;
    win->attrs[BG_B] = b;

    win_set_attr(win, BG_RGB);
}

// -----------------------------------------------------------------------

void win_set_fg(window_t *win, uint8_t color)
{
    win->attrs[FG] = color;
    win_clr_attr(win, FG_RGB | FG_GRAY);
}

// -----------------------------------------------------------------------

void win_set_bg(window_t *win, uint8_t color)
{
    win->attrs[BG] = color;
    win_clr_attr(win, BG_RGB | BG_GRAY);
}

// -----------------------------------------------------------------------

static void win_erase_line(window_t *win, uint16_t line)
{
    uint16_t i;

    cell_t cell;
    cell_t *p = win_line_addr(win, line);

    memcpy(cell.attrs, win->attrs, 8);
    cell.code = win->blank;

    for(i = 0; i < win->width; i++)
    {
        *p++ = cell;
    }
}

// -----------------------------------------------------------------------

void win_clear(window_t *win)
{
    uint16_t i;

    for(i = 0; i != win->height; i++)
    {
        win_erase_line(win, i);
    }
    win->cx = 0;
    win->cy = 0;
}

// -----------------------------------------------------------------------

static void win_copy_line(window_t *win, uint16_t src, uint16_t dst)
{
   cell_t *s = win_line_addr(win, src);
   cell_t *d = win_line_addr(win, dst);

   memcpy(d, s, win->width * sizeof(cell_t));
}

// -----------------------------------------------------------------------

void win_scroll_up(window_t *win)
{
    uint16_t i;

    for(i = 0; i < win->height -1; i++)
    {
        win_copy_line(win, i + 1, i);
    }

    win_erase_line(win, win->height -1);
}

// -----------------------------------------------------------------------

void win_scroll_dn(window_t *win)
{
    uint16_t i;

    for(i = win->height -1; i != 1; i--)
    {
        win_copy_line(win, i - 1, i);
    }

    win_erase_line(win, 0);
}

// -----------------------------------------------------------------------

void win_scroll_lt(window_t *win)
{
    uint16_t i;
    cell_t *s;
    cell_t *d;
    cell_t cell;

    memcpy(cell.attrs, win->attrs, sizeof(cell.attrs));
    cell.code = win->blank;

    for(i = 0; i < win->width; i++)
    {
        s = d = win_line_addr(win, i);
        s++;
        memcpy(d, s, (win->width - 1) * sizeof(cell_t));
        d[win->width -1 ] = cell;
    }
}

// -----------------------------------------------------------------------

void win_scroll_rt(window_t *win)
{
    uint8_t i;
    cell_t *s;
    cell_t *d;
    cell_t cell;

    memcpy(cell.attrs, win->attrs, sizeof(cell.attrs));
    cell.code = win->blank;

    for(i = win->width -1; i != 0; i--)
    {
        s = d = win_line_addr(win, i);
        d++;
        memmove(&d[1], s, (win->width - 1) * sizeof(cell_t));
        s[0] = cell;
    }
}

// -----------------------------------------------------------------------
// set cursor position within window

void win_cup(window_t *win, uint16_t x, uint16_t y)
{
    if((x < win->width) && (y < win->height))
    {
        win->cx = x;
        win->cy = y;
    }
}

// -----------------------------------------------------------------------

void win_set_cx(window_t *win, uint16_t x)
{
    if(x < win->width)
    {
        win->cx = x;
    }
}

// -----------------------------------------------------------------------

void win_set_cy(window_t *win, uint16_t y)
{
    if(y < win->height)
    {
        win->cy = y;
    }
}

// -----------------------------------------------------------------------

void win_crsr_up(window_t *win)
{
    if(0 != win->cy)
    {
        win->cy--;
    }
}

// -----------------------------------------------------------------------

void win_crsr_dn(window_t *win)
{
    if(win->cy != win->height - 1)
    {
        win->cy++;
    }
    else if(0 == (win->attrs[ATTR] & WIN_LOCKED))
    {
        win_scroll_up(win);
    }
}

// -----------------------------------------------------------------------

void win_crsr_lt(window_t *win)
{
    if(0 == win->cx)
    {
        win->cx--;
    }
    else
    {
        if(0 != win->cy)
        {
            win->cy--;
            win->cx = win->width;
        }
    }
}

// -----------------------------------------------------------------------

void win_crsr_rt(window_t *win)
{
    if(win->cx != win->width - 1)
    {
        win->cx++;
    }
    else
    {
        win_crsr_dn(win);
        win->cx = 0;
    }
}

// -----------------------------------------------------------------------

void win_cr(window_t *win)
{
    win->cx = 0;
    win_crsr_dn(win);
}

// -----------------------------------------------------------------------

// should i add tab output processing to windows?
// add a tab width to structure and move cursor to next tab stop?
// p.s. i hate tabs

void win_emit(window_t *win, uint32_t c)
{
    cell_t cell;
    cell_t *p;
    uint32_t width;

    switch(c)
    {
        case     0x0d: win_cr(win);   break;
        case     0x0a: win_cr(win);   break;
        case     0x09:                break;
        default:
            memcpy(cell.attrs, win->attrs, sizeof(cell.attrs));
            cell.code = c;
            p = win_line_addr(win, win->cy);
            p[win->cx] = cell;
            win_crsr_rt(win);
            // need to mark the next cell as used by this character too
            // if this character is double width.  when this is all
            // written to the console later the cell_t's after any wide
            // character are skipped
            width = utf8_encode(c);
            if(1 != width)
            {
                cell.code = 0xaaaaaaaa;
                p[win->cx] = cell;
                win_crsr_rt(win);
            }
    }
}

// =======================================================================
