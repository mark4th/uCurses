// uC_window_attr.c   - set or clear window attributes
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_window.h"

// -----------------------------------------------------------------------

API void uC_win_set_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_flags(&win->attrs, ATTR_FLAG_BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_flags(&win->attrs, ATTR_FLAG_BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_flags(&win->bdr_attrs, ATTR_FLAG_BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_bold(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_flags(&win->bdr_attrs, ATTR_FLAG_BOLD);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_flags(&win->attrs, ATTR_FLAG_REV);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_flags(&win->attrs, ATTR_FLAG_REV);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_flags(&win->bdr_attrs, ATTR_FLAG_REV);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_bdr_rev(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_flags(&win->bdr_attrs, ATTR_FLAG_REV);
    }
}

// -----------------------------------------------------------------------
// if anyone wants to apply an underline to the border they are nuts
// -----------------------------------------------------------------------

API void uC_win_set_ul(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_set_flags(&win->attrs, ATTR_FLAG_UL);
    }
}

// -----------------------------------------------------------------------

API void uC_win_clr_ul(uC_window_t *win)
{
    if (win != NULL)
    {
        uC_attr_clr_flags(&win->attrs, ATTR_FLAG_UL);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        set_fg(&win->attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        set_bg(&win->attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_gray_fg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        set_gray_fg(&win->attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_gray_bg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        set_gray_bg(&win->attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        set_rgb_fg(&win->attrs, r, g, b);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g,
    uC_color_t b)
{
    if (win != NULL)
    {
        set_rgb_bg(&win->attrs, r, g, b);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        set_fg(&win->bdr_attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        set_bg(&win->bdr_attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_gray_fg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        set_gray_fg(&win->bdr_attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_gray_bg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        win->bdr_attrs.bg_gray = color;
        uC_attr_set_flags(&win->bdr_attrs, ATTR_FLAG_GRAY_BG);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rgb_fg(uC_window_t *win, uC_color_t r,
    uC_color_t g, uC_color_t b)
{
    if (win != NULL)
    {
        set_rgb_fg(&win->bdr_attrs, r, g, b);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_bdr_rgb_bg(uC_window_t *win, uC_color_t r,
    uC_color_t g, uC_color_t b)
{
    if (win != NULL)
    {
        set_rgb_bg(&win->bdr_attrs, r, g, b);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_focus_fg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        set_fg(&win->focus_attrs, color);
    }
}


// -----------------------------------------------------------------------

API void uC_win_set_focus_bg(uC_window_t *win, uC_color_t color)
{
    if (win != NULL)
    {
        set_bg(&win->focus_attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_focus_gray_fg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        set_gray_fg(&win->focus_attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_focus_gray_bg(uC_window_t *win, uC_colors_gray_t color)
{
    if (win != NULL)
    {
        set_gray_bg(&win->focus_attrs, color);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_focus_rgb_fg(uC_window_t *win, uC_color_t r,
    uC_color_t g, uC_color_t b)
{
    if (win != NULL)
    {
        set_rgb_fg(&win->focus_attrs, r, g, b);
    }
}

// -----------------------------------------------------------------------

API void uC_win_set_focus_rgb_bg(uC_window_t *win, uC_color_t r,
    uC_color_t g, uC_color_t b)
{
    if (win != NULL)
    {
        set_rgb_bg(&win->focus_attrs, r, g, b);
    }
}

// =======================================================================
