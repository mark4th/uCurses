// window.c
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE // needed to make wcwidth work

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_attribs.h"
#include "uC_borders.h"
#include "uC_win_printf.h"

// -----------------------------------------------------------------------
// calculate address of line within window buffer

static cell_t *win_line_addr(uC_window_t *win, int16_t line)
{
    int16_t index = (win->width) * line;
    return &win->buffer[index];
}

// -----------------------------------------------------------------------
// clear one line of a window

API void uC_win_clear_line(uC_window_t *win, int16_t line)
{
    int16_t i;
    cell_t cell;
    cell_t *p;

    if ((win != NULL) && (line >= 0))
    {
        if (line < win->height)
        {
            p = win_line_addr(win, line);

            cell.attrs = win->attrs;
            cell.code  = win->blank;

            for (i = 0; i < win->width; i++)
            {
                *p++ = cell;
            }
        }
    }
}

// -----------------------------------------------------------------------
// clear entire window

API void uC_win_clear(uC_window_t *win)
{
    int16_t i;

    if (win != NULL)
    {
        for (i = 0; i != win->height; i++)
        {
            uC_win_clear_line(win, i);
        }
        win->cx = 0;
        win->cy = 0;
    }
}

// -----------------------------------------------------------------------
// copy one line of window to another line in same window

static void win_copy_line(uC_window_t *win, int16_t sl, int16_t dl)
{
    cell_t *src = win_line_addr(win, sl);
    cell_t *dst = win_line_addr(win, dl);

    memcpy(dst, src, win->width * sizeof(cell_t));
}

// -----------------------------------------------------------------------

API void uC_win_copy_win(uC_window_t *dst, uC_window_t *src)
{
    int size;

    if ((dst == NULL) || (src == NULL))
    {
        return;
    }

    if (dst == src)
    {
        return;
    }

    if ((dst->width != src->width) &&
        (dst->height!= src->height))
    {
        return;
    }

    size = src->width * src->height * sizeof(cell_t);
    memcpy(dst->buffer, src->buffer, size);
}

// -----------------------------------------------------------------------
// scroll window up by n lines

API void uC_win_scroll_up_n(uC_window_t *win, int16_t n)
{
    int16_t i;

    if (!win) { return; }
    if (n <= 0) { return; }
    if (n >= win->height) { n = win->height; }

    for (i = 0; i < win->height - n; i++)
    {
        win_copy_line(win, i + n, i);
    }

    for (i = win->height - n; i < win->height; i++)
    {
        uC_win_clear_line(win, i);
    }
}

// -----------------------------------------------------------------------
// scroll window down by n lines

API void uC_win_scroll_dn_n(uC_window_t *win, int16_t n)
{
    int16_t i;

    if (!win) { return; }
    if (n <= 0) { return; }
    if (n >= win->height) { n = win->height; }

    for (i = win->height - 1; i >= n; i--)
    {
        win_copy_line(win, i - n, i);
    }

    for (i = 0; i < n; i++)
    {
        uC_win_clear_line(win, i);
    }
}

// -----------------------------------------------------------------------
// pan window left by n columns

API void uC_win_scroll_lt_n(uC_window_t *win, int16_t n)
{
    int16_t i;
    int16_t j;
    cell_t *row;
    cell_t  cell;

    if (!win) { return; }
    if (n <= 0) { return; }
    if (n >= win->width) { n = win->width; }

    cell.attrs = win->attrs;
    cell.code  = win->blank;

    for (i = 0; i < win->height; i++)
    {
        row = win_line_addr(win, i);
        memmove(row, row + n, (win->width - n) * sizeof(cell_t));

        for (j = win->width - n; j < win->width; j++)
        {
            row[j] = cell;
        }
    }
}

// -----------------------------------------------------------------------
// pan window right by n columns

API void uC_win_scroll_rt_n(uC_window_t *win, int16_t n)
{
    int16_t i;
    int16_t j;
    cell_t *row;
    cell_t  cell;

    if (!win) { return; }
    if (n <= 0) { return; }
    if (n >= win->width) { n = win->width; }

    cell.attrs = win->attrs;
    cell.code  = win->blank;

    for (i = 0; i < win->height; i++)
    {
        row = win_line_addr(win, i);
        memmove(row + n, row, (win->width - n) * sizeof(cell_t));

        for (j = 0; j < n; j++)
        {
            row[j] = cell;
        }
    }
}

// -----------------------------------------------------------------------

API void uC_win_scroll_up(uC_window_t *win) { uC_win_scroll_up_n(win, 1); }
API void uC_win_scroll_dn(uC_window_t *win) { uC_win_scroll_dn_n(win, 1); }
API void uC_win_scroll_lt(uC_window_t *win) { uC_win_scroll_lt_n(win, 1); }
API void uC_win_scroll_rt(uC_window_t *win) { uC_win_scroll_rt_n(win, 1); }

// -----------------------------------------------------------------------
// set cursor position within window

API void uC_win_cup(uC_window_t *win, int16_t x, int16_t y)
{
    if ((win != NULL) && (x >= 0) && (y >= 0))
    {
        if ((x < win->width) && (y < win->height))
        {
            win->cx = x;
            win->cy = y;
        }
    }
}

// -----------------------------------------------------------------------
// set cursor x position window window

API void uC_win_set_cx(uC_window_t *win, int16_t x)
{
    if ((win != NULL) && (x >= 0) && (x < win->width))
    {
        win->cx = x;
    }
}

// -----------------------------------------------------------------------
// set cursor y position within window

API void uC_win_set_cy(uC_window_t *win, int16_t y)
{
    if ((win != NULL) && (y >= 0) && (y < win->height))
    {
        win->cy = y;
    }
}

// -----------------------------------------------------------------------
// move cursor up one line in window

API void uC_win_crsr_up(uC_window_t *win)
{
    if ((win != NULL) && (win->cy != 0))
    {
        win->cy--;
    }
}

// -----------------------------------------------------------------------
// move cursor down one line in window

API void uC_win_crsr_dn(uC_window_t *win)
{
    if (win != NULL)
    {
        if (win->cy != win->height - 1)
        {
            win->cy++;
        }
        else if ((win->flags & uC_WIN_LOCKED) == 0)
        {
            uC_win_scroll_up(win);
        }
    }
}

// -----------------------------------------------------------------------
// move cursor left one column in window

API void uC_win_crsr_lt(uC_window_t *win)
{
    if (win != NULL)
    {
        if (win->cx != 0)
        {
            win->cx--;
        }
        else if (win->cy != 0)
        {
            win->cy--;
            win->cx = win->width;
        }
    }
}

// -----------------------------------------------------------------------
// move cursor right one column in window

API void uC_win_crsr_rt(uC_window_t *win)
{
    if (win != NULL)
    {
        if (win->cx != win->width - 1)
        {
            win->cx++;
        }
        else
        {
            if ((win->flags & uC_WIN_LOCKED) == 0)
            {
                uC_win_crsr_dn(win);
            }
            win->cx = 0;
        }
    }
}

// -----------------------------------------------------------------------
// move cursor to start of next line (may cause a scroll)

API void uC_win_cr(uC_window_t *win)
{
    if (win != NULL)
    {
        win->cx = 0;
        uC_win_crsr_dn(win);
    }
}

// -----------------------------------------------------------------------

static void _win_emit(uC_window_t *win, uint32_t c)
{
    cell_t cell;
    cell_t *p;
    int32_t width;

    width = wcwidth(c);

    if (width <= 0)
    {
        return;
    }

    // auto carriage return should be an optional setting
    if ((win->cx + width) > win->width)
    {
        uC_win_cr(win);
    }

    cell.attrs = win->attrs;
    cell.code  = c;

    p = win_line_addr(win, win->cy);
    p[win->cx] = cell;

    uC_win_crsr_rt(win);
    width--;

    // different utf8 characters are displayed using various
    // numbers of columns on the display - most are only one or
    // two columns wide but there are arabic characters for example
    // that take 4 or 5 columns.  every column after the first
    // needs to be marked as used by this character too

    cell.code = DEADC0DE;

    while (width != 0)
    {
        // if the left edge of this double wide character is
        // later overwritten with a single wide character
        // we will need to draw a space over the DEADC0DE
        // with the same attributes

        p[win->cx] = cell;
        uC_win_crsr_rt(win);
        width--;
    }
}

// -----------------------------------------------------------------------

API void uC_win_emit(uC_window_t *win, uint32_t c)
{
    if (win != NULL)
    {
        if (c != 0x09)      // tabs are evil kthxbai
        {
            ((c == 0x0d) || (c == 0x0a))
                ? uC_win_cr(win)
                : _win_emit(win, c);
        }
    }
}

// -----------------------------------------------------------------------

API cell_t *uC_win_peek_xy(uC_window_t *win, int16_t x, int16_t y)
{
    cell_t *p;

    if ((x < win->width) && (y < win->height) &&
        (x >= 0) && (y >= 0))
    {
        p = win_line_addr(win, y);
        return &p[x];
    }
    return NULL;
}

// -----------------------------------------------------------------------

API cell_t *uC_win_peek(uC_window_t *win)
{
    return uC_win_peek_xy(win, win->cx, win->cy);
}

// -----------------------------------------------------------------------
// erase to end of line

API void uC_win_el(uC_window_t *win)
{
    if (win != NULL)
    {
        do
        {
            uC_win_emit(win, win->blank);
        } while (win->cx != 0);
    }
}

// =======================================================================
