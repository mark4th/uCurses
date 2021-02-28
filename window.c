// window.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h/color.h"
#include "h/tui.h"

// -----------------------------------------------------------------------

#define win_line_addr(win, line) &win->buffer[win->width * line];

// -----------------------------------------------------------------------
// allocate buffer for window contents

static bool win_alloc(window_t *win)
{
    void *p;

    uint32_t size = ((win->width + win->height) * sizeof(cell_t));
    p = malloc(size);

    if(NULL != p)
    {
        win->buffer = p;
        return true;
    }
    return false;
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

void win_open(uint16_t width, uint16_t height, window_t *win)
{
    memset(win, 0, sizeof(window_t));
    win->height  = height;
    win->width   = width;
    memset(win->attrs, 0, sizeof(win->attrs));
    win->attrs[FG] = WHITE;
    win->attrs[BG] = BLACK;
    win->blank   = 0x20;
    win_alloc(win);
}

// -----------------------------------------------------------------------
// set new x/y position of window within parent screen

bool win_move(window_t *win, uint16_t x, uint16_t y)
{
    uint16_t scr_width, scr_height;
    uint16_t win_width, win_height;
    uint16_t win_x, win_y;

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

        return true;
    }

    return false;
}

// -----------------------------------------------------------------------

void win_set_attr(window_t *win, ti_attrib_t attr)
{
    win->attrs[ATTR] |= attr;

    // gray scales and rgb are mutually exclusive
    if(FG_RGB == attr)   { win->attrs[ATTR] &= ~FG_GRAY; }
    if(BG_RGB == attr)   { win->attrs[ATTR] &= ~BG_GRAY; }
    if(FG_GRAY == attr)  { win->attrs[ATTR] &= ~FG_RGB;  }
    if(BG_GRAY == attr)  { win->attrs[ATTR] &= ~BG_RGB;  }
}

// -----------------------------------------------------------------------

#define win_clr_attr(win, attr) win->attrs[ATTR] &= ~attr

// -----------------------------------------------------------------------

#define win_set_ul(win)    win_set_attr(win, UNDERLINE)
#define win_set_rev(win)   win_set_attr(win, REVERSE)
#define win_set_bold(win)  win_set_attr(win, BOLD)
#define win_set_blink(win) win_set_attr(win, BLINK)

// -----------------------------------------------------------------------

#define win_clr_ul(win)    win_clr_attr(win, UNDERLINE)
#define win_clr_rev(win)   win_clr_attr(win, REVERSE)
#define win_clr_bold(win)  win_clr_attr(win, BOLD)
#define win_clr_blink(win) win_clr_attr(win, BLINK)

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

#define win_set_boxed(win)   win->flags |= WIN_BOXED
#define win_set_locked(win)  win->flags |= WIN_LOCKED
#define win_set_filled(win)  win->flags |= WIN_FILLED

#define win_clr_boxed(win)   win->flags &= ~WIN_BOXED
#define win_clr_locked(win)  win->flags &= ~WIN_LOCKED
#define win_clr_filled(win)  win->flags &= ~WIN_FILLED

// -----------------------------------------------------------------------

void win_erase_line(window_t *win, uint16_t line)
{
    uint16_t i;

    cell_t cell;
    cell_t *p = win_line_addr(win, line);

    memcpy(cell.attrs, win->attrs, sizeof(cell.attrs));
    cell.code = win->blank;

    for(i = 0; i < win->width; i++)
    {
        p[i] = cell;
    }
}

// -----------------------------------------------------------------------

void win_copy_line(window_t *win, uint16_t src, uint16_t dst)
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

    cell_t *d;
    cell_t *s;

    for(i = win->height -1; i != 0; i--)
    {
        s = d = win_line_addr(win, i);
        s++;
        memcpy(d, s, (win->width - 1) * sizeof(cell_t));
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

bool win_cup(window_t *win, uint16_t x, uint16_t y)
{
    if((x < win->width) && (y < win->height))
    {
        win->cx = x;
        win->cy = y;
        return true;
    }
    return false;
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
    else
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

void do_win_emit(window_t *win, wchar_t c)
{
    cell_t cell;

    memcpy(cell.attrs, win->attrs, sizeof(cell.attrs));
    cell.code = c;

    cell_t *p = win_line_addr(win, win->cy);
    p[win->cx] = cell;
    win_crsr_rt(win);
}

// -----------------------------------------------------------------------

// should i add tab output processing to windows?
// add a tab width to structure and move cursor to next tab stop?
// p.s. i hate tabs

void win_emit(window_t *win, wchar_t c)
{
    switch(c)
    {
        case     0x0d: win_cr(win);   break;
        case     0x0a:                break;
        case     0x09:                break;
        default: do_win_emit(win, c); break;
    }
}

// -----------------------------------------------------------------------

void win_clear(window_t *win)
{
    uint16_t i;
    for(i = 0; i < win->height - 1; i++)
    {
        win_erase_line(win, i);
    }
    win->cx = 0;
    win->cy = 0;
}

// -----------------------------------------------------------------------
// draw specified window line into its parent screen buffer 1

static void win_draw_row(window_t *win, uint16_t line)
{
    screen_t *scr = win->screen;
    cell_t *dst = &scr->buffer1[0];
    cell_t *src = win_line_addr(win, line);
    dst += win->cx;
    memcpy(dst, src, (win->width - 1) * sizeof(cell_t));
}

// -----------------------------------------------------------------------

void win_draw(window_t *win)
{
    uint16_t i;

    for(i = 0; i < win->height - 1; i++)
    {
        win_draw_row(win, i);
    }
}

// =======================================================================
