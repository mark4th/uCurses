// window.c
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE // needed to make wcwidth work

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_attribs.h"
#include "uC_borders.h"

// -----------------------------------------------------------------------
// calculate address of line within window buffer

static cell_t *win_line_addr(uC_window_t *win, int16_t line)
{
    int16_t index = (win->width) * line;
    return &win->buffer[index];
}

// -----------------------------------------------------------------------
// allocate buffer for window contents

int16_t win_alloc(uC_window_t *win)
{
    int16_t rv = -1;        // assume failure
    cell_t *p;

    if (win != NULL)
    {
        p = calloc((win->width * win->height), sizeof(cell_t));

        if (p != NULL)
        {
            win->buffer = p;
            rv = 0;
        }
    }

    return rv;
}

// -----------------------------------------------------------------------
// close a window and free all associated allocated resources

API void uC_win_close(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_scr_win_detach(win);
        if (win->buffer != NULL)
        {
            free(win->buffer);
        }
        free(win);
    }
}

// -----------------------------------------------------------------------
// open a new window of given dimensions

API uC_window_t *uC_win_open(int16_t width, int16_t height)
{
    uC_window_t *win = calloc(1, sizeof(*win));

    if (win != NULL)
    {
        win->width  = width;
        win->height = height;

        // win_alloc() uses width/height to determine how much
        // space needs to be allocated for the buffers

        if (win_alloc(win) == 0)
        {
            win->attrs.bytes[FG] = DEFAULT_FG;
            win->attrs.bytes[BG] = DEFAULT_BG;
            uC_win_clear(win);
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
// pop window to front in view

API void uC_win_pop(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_screen_t *scr = win->screen;
        uC_scr_win_detach(win);
        uC_scr_win_attach(scr, win);
    }
}

// -----------------------------------------------------------------------
// push window to back

API void uC_win_push(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_screen_t *scr = win->screen;
        uC_scr_win_detach(win);

        if (uC_list_push_head(&scr->windows, win) != true)
        {
            // oopts
            // internal error?
            // planetary alignment issue?
        }
    }
}

// -----------------------------------------------------------------------
// set new x/y position of window within parent screen

API int16_t uC_win_set_pos(uC_window_t *win, int16_t x, int16_t y)
{
    int16_t scr_width, scr_height;
    int16_t win_width, win_height;
    int16_t win_x, win_y;
    int16_t rv = -1;

    if ((win != NULL) && (x >= 0) && (y >= 0))
    {
        uC_screen_t *scr = win->screen;

        if (scr == NULL)
        {
            return rv;
        }

        win_width  = win->width;
        win_height = win->height;
        win_x      = x;
        win_y      = y;
        scr_width  = scr->width;
        scr_height = scr->height;

        // if window is boxed account for border
        if (win->flags & WIN_BOXED)
        {
            win_width  += 2;
            win_height += 2;
            win_x--;
            win_y--;
        }

        if ((win_x + win_width < scr_width) &&
           (win_y  + win_height < scr_height))
        {
            win->xco = x;
            win->yco = y;

            rv = 0;
        }
    }
    return rv;
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

            cell.attrs.chunk = win->attrs.chunk;
            cell.code        = win->blank;

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
    if (win != NULL)
    {
        cell_t *src = win_line_addr(win, sl);
        cell_t *dst = win_line_addr(win, dl);

        memcpy(dst, src, win->width * sizeof(cell_t));
    }
}

// -----------------------------------------------------------------------
// scroll window up one line

API void uC_win_scroll_up(uC_window_t *win)
{
    int16_t i;

    if (win != NULL)
    {
        for (i = 0; i < win->height - 1; i++)
        {
            win_copy_line(win, i + 1, i);
        }

        uC_win_clear_line(win, win->height - 1);
    }
}

// -----------------------------------------------------------------------
// scroll widnow down one line

API void uC_win_scroll_dn(uC_window_t *win)
{
    int16_t i;

    if (win != NULL)
    {
        for (i = win->height - 1; i != 1; i--)
        {
            win_copy_line(win, i - 1, i);
        }

        uC_win_clear_line(win, 0);
    }
}

// -----------------------------------------------------------------------
// pan window left one column

API void uC_win_scroll_lt(uC_window_t *win)
{
    int16_t i;

    cell_t *src;
    cell_t *dst;
    cell_t cell;

    if (win != NULL)
    {
        cell.attrs.chunk = win->attrs.chunk;
        cell.code = win->blank;

        for (i = 0; i < win->height; i++)
        {
            src = dst = win_line_addr(win, i);
            src++;
            memcpy(dst, src, (win->width - 1) * sizeof(cell_t));
            dst[win->width - 1] = cell;
        }
    }
}

// -----------------------------------------------------------------------
// pan window right one column

API void uC_win_scroll_rt(uC_window_t *win)
{
    int8_t i;

    cell_t *src;
    cell_t *dst;
    cell_t cell;

    if (win != NULL)
    {
        cell.attrs.chunk = win->attrs.chunk;
        cell.code = win->blank;

        for (i = 0; i < win->height; i++)
        {
            src = dst = win_line_addr(win, i);
            dst++;
            memmove(&dst[1], src, (win->width - 1) * sizeof(cell_t));
            src[0] = cell;
        }
    }
}

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
        else if ((win->flags & WIN_LOCKED) == 0)
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
        if (win->cx == 0)
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
            if ((win->flags & WIN_LOCKED) == 0)
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

static void _win_emit(uC_window_t *win, int32_t c)
{
    cell_t cell;
    cell_t *p;
    int32_t width;

    width = wcwidth(c);

    if (width <= 0)
    {
        return;
    }

    if ((win->cx + width) > win->width)
    {
        uC_win_cr(win);
    }

    cell.attrs.chunk = win->attrs.chunk;
    cell.code = c;

    p = win_line_addr(win, win->cy);
    p[win->cx] = cell;

    uC_win_crsr_rt(win);
    width--;

// redo

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

API void uC_win_emit(uC_window_t *win, int32_t c)
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

// -----------------------------------------------------------------------

API void uC_win_set_flag(uC_window_t *win, win_flags_t flag)
{
    if (win != NULL)
    {
        win->flags |= flag;
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_flag(uC_window_t *win, win_flags_t flag)
{
    if (win != NULL)
    {
        win->flags &= ~flag;
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attrs, BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attrs, BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attrs, REVERSE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attrs, REVERSE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_ul(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attrs, UNDERLINE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_ul(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attrs, UNDERLINE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->bdr_attrs, BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->bdr_attrs, BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->bdr_attrs, REVERSE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->bdr_attrs, REVERSE);
    }
}

// -----------------------------------------------------------------------
// would anyone ever actually want a winder border underlined?

// API void uC_win_set_bdr_ul(uC_window_t *win)
// {
//     if (win != NULL)
//     {
//         uC_attr_set_attr(&win->bdr_attrs, UNDERLINE);
//     }
// }

// -----------------------------------------------------------------------

// API void uC_win_clr_bdr_ul(uC_window_t *win)
// {
//     if (win != NULL)
//     {
//         uC_attr_clr_attr(&win->bdr_attrs, UNDERLINE);
//     }
// }

// -----------------------------------------------------------------------

API void uC_win_set_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attrs, FG, color);
        uC_attr_clr_attr(&win->attrs, FG_RGB | FG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attrs, BG, color);
        uC_attr_clr_attr(&win->attrs, BG_RGB | BG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_gray_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attrs, FG, color);
        uC_attr_set_attr(&win->attrs, FG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_gray_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attrs, BG, color);
        uC_attr_set_attr(&win->attrs, BG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attrs, FG_R, r);
        uC_attr_set_bytes(&win->attrs, FG_G, g);
        uC_attr_set_bytes(&win->attrs, FG_B, b);
        uC_attr_set_attr(&win->attrs, FG_RGB);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attrs, BG_R, r);
        uC_attr_set_bytes(&win->attrs, BG_G, g);
        uC_attr_set_bytes(&win->attrs, BG_B, b);
        uC_attr_set_attr(&win->attrs, BG_RGB);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->bdr_attrs, FG, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->bdr_attrs, BG, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_gray_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->bdr_attrs, FG, color);
        uC_attr_set_attr(&win->bdr_attrs, FG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_gray_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->bdr_attrs, BG, color);
        uC_attr_set_attr(&win->bdr_attrs, BG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->bdr_attrs, FG_R, r);
        uC_attr_set_bytes(&win->bdr_attrs, FG_G, g);
        uC_attr_set_bytes(&win->bdr_attrs, FG_B, b);

        uC_attr_set_attr(&win->bdr_attrs, FG_RGB);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->bdr_attrs, BG_R, r);
        uC_attr_set_bytes(&win->bdr_attrs, BG_G, g);
        uC_attr_set_bytes(&win->bdr_attrs, BG_B, b);

        uC_attr_set_attr(&win->bdr_attrs, BG_RGB);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_border(uC_window_t *win, uint16_t border_type)
{
    win->border_type = border_type;
}

// =======================================================================
