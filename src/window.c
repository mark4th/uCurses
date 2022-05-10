// window.c
// -----------------------------------------------------------------------

#define _XOPEN_SOURCE // needed to make wcwidth work

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "uCurses.h"
#include "window.h"
#include "screen.h"
#include "attribs.h"

// -----------------------------------------------------------------------
// calculate address of line within window buffer

static cell_t *win_line_addr(window_t *win, int16_t line)
{
    int16_t index = (win->width) * line;
    return &win->buffer[index];
}

// -----------------------------------------------------------------------
// allocate buffer for window contents

int16_t win_alloc(window_t *win)
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

API void uC_win_close(void *win)
{
    if (win != NULL)
    {
        if (((window_t *)win)->buffer != NULL)
        {
            free(((window_t *)win)->buffer);
        }
        free(win);
    }
}

// -----------------------------------------------------------------------

API window_t *uC_win_open(int16_t width, int16_t height)
{
    window_t *win = calloc(1, sizeof(*win));

    if (win != NULL)
    {
        win->width  = width;
        win->height = height;

        // win_alloc() uses width/height to determine how much
        // space needs to be allocated for the buffers

        if (win_alloc(win) == 0)
        {
            win->attr_grp.attrs.bytes[FG] = DEFAULT_FG;
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
// pop window to front

API void uC_win_pop(window_t *win)
{
    if (win != NULL)
    {
        screen_t *scr = win->screen;
        uC_scr_win_detach(win);
        uC_scr_win_attach(scr, win);
    }
}

// -----------------------------------------------------------------------
// set new x/y position of window within parent screen

API int16_t uC_win_set_pos(window_t *win, int16_t x, int16_t y)
{
    int16_t scr_width, scr_height;
    int16_t win_width, win_height;
    int16_t win_x, win_y;
    int16_t rv = -1;

    if (win != NULL)
    {
        screen_t *scr = win->screen;

        win_width = win->width;
        win_height = win->height;
        win_x = x;
        win_y = y;
        scr_width = scr->width;
        scr_height = scr->height;

        // if window is boxed account for border
        if (win->flags & WIN_BOXED)
        {
            win_width += 2;
            win_height += 2;
            win_x--;
            win_y--;
        }

        if ((win_x + win_width < scr_width) &&
           (win_y + win_height < scr_height))
        {
            win->xco = x;
            win->yco = y;

            rv = 0;
        }
    }
    return rv;
}

// -----------------------------------------------------------------------

API void uC_win_erase_line(window_t *win, int16_t line)
{
    int16_t i;
    cell_t cell;
    cell_t *p;

    if (win != NULL)
    {
        p = win_line_addr(win, line);

        cell.attrs.chunk = win->attr_grp.attrs.chunk;
        cell.code = win->blank;

        for (i = 0; i < win->width; i++)
        {
            *p++ = cell;
        }
    }
}

// -----------------------------------------------------------------------
// clear entire window

API void uC_win_clear(window_t *win)
{
    int16_t i;

    if (win != NULL)
    {
        for (i = 0; i != win->height; i++)
        {
            uC_win_erase_line(win, i);
        }
        win->cx = 0;
        win->cy = 0;
    }
}

// -----------------------------------------------------------------------

static void win_copy_line(window_t *win, int16_t sl, int16_t dl)
{
    cell_t *src = win_line_addr(win, sl);
    cell_t *dst = win_line_addr(win, dl);

    memcpy(dst, src, win->width * sizeof(cell_t));
}

// -----------------------------------------------------------------------

API void uC_win_scroll_up(window_t *win)
{
    int16_t i;

    if (win != NULL)
    {
        for (i = 0; i < win->height - 1; i++)
        {
            win_copy_line(win, i + 1, i);
        }

        uC_win_erase_line(win, win->height - 1);
    }
}

// -----------------------------------------------------------------------

API void uC_win_scroll_dn(window_t *win)
{
    int16_t i;

    if (win != NULL)
    {
        for (i = win->height - 1; i != 1; i--)
        {
            win_copy_line(win, i - 1, i);
        }

        uC_win_erase_line(win, 0);
    }
}

// -----------------------------------------------------------------------

API void uC_win_scroll_lt(window_t *win)
{
    int16_t i;

    cell_t *src;
    cell_t *dst;
    cell_t cell;

    if (win != NULL)
    {
        cell.attrs.chunk = win->attr_grp.attrs.chunk;

        cell.code = win->blank;

        for (i = 0; i < win->width; i++)
        {
            src = dst = win_line_addr(win, i);
            src++;
            memcpy(dst, src, (win->width - 1) * sizeof(cell_t));
            dst[win->width - 1] = cell;
        }
    }
}

// -----------------------------------------------------------------------

API void uC_win_scroll_rt(window_t *win)
{
    int8_t i;

    cell_t *src;
    cell_t *dst;
    cell_t cell;

    if (win != NULL)
    {
        cell.attrs.chunk = win->attr_grp.attrs.chunk;
        cell.code = win->blank;

        for (i = win->width - 1; i != 0; i--)
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

API void uC_win_cup(window_t *win, int16_t x, int16_t y)
{
    if (win != NULL)
    {
        if ((x < win->width) && (y < win->height))
        {
            win->cx = x;
            win->cy = y;
        }
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_cx(window_t *win, int16_t x)
{
    if ((win != NULL) && (x < win->width))
    {
        win->cx = x;
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_cy(window_t *win, int16_t y)
{
    if ((win != NULL) && (y < win->height))
    {
        win->cy = y;
    }
}

// -----------------------------------------------------------------------

API void uC_win_crsr_up(window_t *win)
{
    if ((win != NULL) && (win->cy != 0))
    {
        win->cy--;
    }
}

// -----------------------------------------------------------------------

API void uC_win_crsr_dn(window_t *win)
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

API void uC_win_crsr_lt(window_t *win)
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

API void uC_win_crsr_rt(window_t *win)
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

API void uC_win_cr(window_t *win)
{
    if (win != NULL)
    {
        win->cx = 0;
        uC_win_crsr_dn(win);
    }
}

// -----------------------------------------------------------------------

static void _win_emit(window_t *win, uint32_t c)
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

    cell.attrs.chunk = win->attr_grp.attrs.chunk;
    cell.code = c;

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

API void uC_win_emit(window_t *win, int32_t c)
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

API cell_t *uC_win_peek_xy(window_t *win, uint16_t x, uint16_t y)
{
    cell_t *p;
    p = win_line_addr(win, y);
    return &p[x];
}

// -----------------------------------------------------------------------

API cell_t *uC_win_peek(window_t *win)
{
    return uC_win_peek_xy(win, win->cx, win->cy);
}

// -----------------------------------------------------------------------
// erase to end of line

API void uC_win_el(window_t *win)
{
    if(win != NULL)
    {
        do
        {
            uC_win_emit(win, win->blank);
        } while (win->cx != 0);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_flag(window_t *win, win_flags_t flag)
{
    if (win != NULL)
    {
        win->flags |= flag;
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_flag(window_t *win, win_flags_t flag)
{
    if (win != NULL)
    {
        win->flags &= ~flag;
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bold(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attr_grp.attrs, BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bold(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attr_grp.attrs, BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rev(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attr_grp.attrs, REVERSE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_rev(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attr_grp.attrs, REVERSE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_ul(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attr_grp.attrs, UNDERLINE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_ul(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attr_grp.attrs, UNDERLINE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_bold(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attr_grp.bdr_attrs, BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_bold(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attr_grp.bdr_attrs, BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rev(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attr_grp.bdr_attrs, REVERSE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_rev(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attr_grp.bdr_attrs, REVERSE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_ul(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attr_grp.bdr_attrs, UNDERLINE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_ul(window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attr_grp.bdr_attrs, UNDERLINE);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_fg(window_t *win, color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.attrs, FG, color);
        uC_attr_clr_attr(&win->attr_grp.attrs, FG_RGB | FG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bg(window_t *win, color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.attrs, BG, color);
        uC_attr_clr_attr(&win->attr_grp.attrs, BG_RGB | BG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_gray_fg(window_t *win, color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.attrs, FG, color);
        uC_attr_set_attr(&win->attr_grp.attrs, FG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_gray_bg(window_t *win, color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.attrs, BG, color);
        uC_attr_set_attr(&win->attr_grp.attrs, BG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rgb_fg(window_t *win, color_t r, color_t g, color_t b)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.attrs, FG_R, r);
        uC_attr_set_bytes(&win->attr_grp.attrs, FG_G, g);
        uC_attr_set_bytes(&win->attr_grp.attrs, FG_B, b);
        uC_attr_set_attr(&win->attr_grp.attrs, FG_RGB);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rgb_bg(window_t *win, color_t r, color_t g, color_t b)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.attrs, BG_R, r);
        uC_attr_set_bytes(&win->attr_grp.attrs, BG_G, g);
        uC_attr_set_bytes(&win->attr_grp.attrs, BG_B, b);
        uC_attr_set_attr(&win->attr_grp.attrs, BG_RGB);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_fg(window_t *win, color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, FG, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_bg(window_t *win, color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, BG, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_gray_fg(window_t *win, color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, FG, color);
        uC_attr_set_attr(&win->attr_grp.bdr_attrs, FG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_gray_bg(window_t *win, color_t color)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, BG, color);
        uC_attr_set_attr(&win->attr_grp.bdr_attrs, BG_GRAY);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rgb_fg(window_t *win, color_t r, color_t g, color_t b)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, FG_R, r);
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, FG_G, g);
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, FG_B, b);
        uC_attr_set_attr(&win->attr_grp.bdr_attrs, FG_RGB);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rgb_bg(window_t *win, color_t r, color_t g, color_t b)
{
    if (win != NULL)
    {
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, BG_R, r);
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, BG_G, g);
        uC_attr_set_bytes(&win->attr_grp.bdr_attrs, BG_B, b);
        uC_attr_set_attr(&win->attr_grp.bdr_attrs, BG_RGB);
    }
}

// =======================================================================
