// uC_window_attr.c   - set or clearn window attributes
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_window.h"

// -----------------------------------------------------------------------

API void uC_win_set_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attrs, ATTR_FLAG_BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attrs, ATTR_FLAG_BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attrs, ATTR_FLAG_REV);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attrs, ATTR_FLAG_REV);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_ul(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->attrs, ATTR_FLAG_UL);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_ul(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->attrs, ATTR_FLAG_UL);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        win->attrs.fg = color;
        uC_attr_clr_attr(&win->attrs, (ATTR_FLAG_RGB_FG | ATTR_FLAG_GRAY_FG));
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        win->attrs.bg = color;
        uC_attr_clr_attr(&win->attrs, (ATTR_FLAG_RGB_BG | ATTR_FLAG_GRAY_BG));
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_gray_fg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        win->attrs.fg_gray = color;
        uC_attr_set_attr(&win->attrs, ATTR_FLAG_GRAY_FG);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_gray_bg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        win->attrs.bg_gray = color;
        uC_attr_set_attr(&win->attrs, ATTR_FLAG_GRAY_BG);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        win->attrs.fg_r = r;
        win->attrs.fg_g = g;
        win->attrs.fg_b = b;
        uC_attr_set_attr(&win->attrs, ATTR_FLAG_RGB_FG);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        win->attrs.bg_r = r;
        win->attrs.bg_g = g;
        win->attrs.bg_b = b;
        uC_attr_set_attr(&win->attrs, ATTR_FLAG_RGB_BG);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->bdr_attrs, ATTR_FLAG_BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->bdr_attrs, ATTR_FLAG_BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_attr(&win->bdr_attrs, ATTR_FLAG_REV);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_attr(&win->bdr_attrs, ATTR_FLAG_REV);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        win->bdr_attrs.fg = color;
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        win->bdr_attrs.bg = color;
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_gray_fg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        win->bdr_attrs.fg_gray = color;
        uC_attr_set_attr(&win->bdr_attrs, ATTR_FLAG_GRAY_FG);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_gray_bg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        win->bdr_attrs.bg_gray = color;
        uC_attr_set_attr(&win->bdr_attrs, ATTR_FLAG_GRAY_BG);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        win->bdr_attrs.fg_r = r;
        win->bdr_attrs.fg_g = g;
        win->bdr_attrs.fg_b = b;

        uC_attr_set_attr(&win->bdr_attrs, ATTR_FLAG_RGB_FG);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        win->bdr_attrs.bg_r = r;
        win->bdr_attrs.bg_g = g;
        win->bdr_attrs.bg_b = b;

        uC_attr_set_attr(&win->bdr_attrs, ATTR_FLAG_RGB_BG);
    }
}

// =======================================================================
