// uC_widget_draw.c
// -----------------------------------------------------------------------

#include <string.h>

#include "uC_widgets.h"
#include "uC_window.h"
#include "uC_utils.h"
#include "uC_list.h"
#include "uC_win_printf.h"
#include "uC_utf8.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;
extern border_t *const borders[];

extern bool justify;

// -----------------------------------------------------------------------
// for radio button and check box widgets

// The order of each of these arrays is as per the
// uC_radio_type_t enumeration in uC_widgets.h

uint16_t radio_off[] =
{   //   ☐       ☐       □       □       ▫       ◇
    0x25fb, 0x25fb, 0x25a1, 0x25a1, 0x25ab, 0x25c7,
    //   △       ▽       ◁       ▷
    0x25b3, 0x25bd, 0x25c1, 0x25b7,
    //   ▵       ▿       ◃       ▹
    0x25b5, 0x25bf, 0x25c3, 0x25b9
};

uint16_t radio_on[]  =
{   //   ☑       ☒       ▣       ■       ▪       ◈
    0x2611, 0x2612, 0x25a3, 0x25a0, 0x25aa, 0x25c8,
    //   ▲       ▼       ◀       ▶
    0x25b2, 0x25bc, 0x25c0, 0x25b6,
    //   ▴       ▾       ◂       ▸
    0x25b4, 0x25be, 0x25c2, 0x25b8
};

// -----------------------------------------------------------------------

void widget_set_attrs(uC_window_t *win, uC_widget_t *widget)
{
    uC_attribs_t attr;
    uC_widget_view_t *view;

    attr = widget->attrs;
    view = widget->view;

    if (widget->focused)
    {
        attr = widget->focus_attrs;

        if (view != widget_state.view)
        {
            uC_set_gray_fg(&attr, uC_GRAY_23);
            uC_set_gray_bg(&attr, uC_GRAY_06);
        }
    }

    win->attrs = attr;
}

// -----------------------------------------------------------------------

uint16_t widget_clear_width(uC_window_t *win,
    uint16_t x, uint16_t y, uint16_t width)
{
    uint16_t clipped;
    uint16_t remaining;

    if ((win == NULL) || (x >= win->width) || (y >= win->height))
    {
        return 0;
    }

    clipped = (uint16_t)(win->width - x);
    if (width < clipped)
    {
        clipped = width;
    }

    uC_win_cup(win, x, y);
    remaining = clipped;
    while (remaining != 0)
    {
        if (!widget_emit_clipped(win, 0x20, &remaining))
        {
            break;
        }
    }
    uC_win_cup(win, x, y);
    return clipped;
}

// -----------------------------------------------------------------------

bool widget_emit_clipped(uC_window_t *win, uint32_t codepoint,
    uint16_t *remaining)
{
    utf8_encode_t *encoded;
    uint16_t line_remaining;
    uint16_t allowed;
    int width;

    if ((win == NULL) || (remaining == NULL) || (*remaining == 0) ||
        (win->cx < 0) || (win->cy < 0) ||
        (win->cx >= win->width) || (win->cy >= win->height))
    {
        return false;
    }

    encoded = uC_utf8_encode((int32_t)codepoint);
    width = encoded->width;
    if (width <= 0)
    {
        return true;
    }

    line_remaining = (uint16_t)(win->width - win->cx);
    allowed = (*remaining < line_remaining) ? *remaining : line_remaining;
    if ((uint16_t)width > allowed)
    {
        return false;
    }

    uC_win_emit(win, codepoint);
    *remaining = (uint16_t)(*remaining - (uint16_t)width);
    return true;
}

// -----------------------------------------------------------------------

void widget_puts_clipped(uC_window_t *win, const char *text,
    uint16_t *remaining)
{
    uint32_t codepoint;
    uint8_t len;

    if ((text == NULL) || (remaining == NULL))
    {
        return;
    }

    while ((*text != '\0') && (*remaining != 0))
    {
        len = uC_utf8_decode(&codepoint, (uint8_t *)text);
        if ((len == 0) || (len > 4))
        {
            break;
        }
        if (!widget_emit_clipped(win, codepoint, remaining))
        {
            break;
        }
        text += len;
    }
}

// -----------------------------------------------------------------------

static void draw_widget(uC_window_t *win, uC_widget_t *widget,
    uint16_t xco, uint16_t yco)
{
    uint16_t x = (xco + widget->xco);
    uint16_t y = (yco + widget->yco);

    // widget coordinates are relative to the view position within its
    // parent window. position the cursor to the widgets location.

    // %@ set cursor x / y location within window

    uC_win_printf(win, "%@", UC_XY(x, y));

    uC_attribs_t save = win->attrs;

    switch (widget->type)
    {
        case uC_WIDGET_NONE:    uC_noop();                       break;
        case uC_WIDGET_BUTTON:  draw_button(win, widget,  x, y); break;
        case uC_WIDGET_RADIO:   draw_radio(win, widget,   x, y); break;
        case uC_WIDGET_CHECK:   draw_check(win, widget,   x, y); break;
        case uC_WIDGET_TEXTBOX: draw_textbox(win, widget, x, y); break;
    }

    win->attrs = save;
}

// -----------------------------------------------------------------------

static void draw_view_box(uC_window_t *win, uC_widget_view_t *view)
{
    border_t *b;

    uC_win_draw_box(win, view->xco - 1, view->yco - 1,
        view->width, view->height,
        view->box_type, view->box_attrs);

    if (view->name)
    {
        b = borders[view->box_type];

        // %@ set cursor x / y location within window
        // %8 emit single utf8 character
        // %s write a string
        // %8 emit single utf8 character

        uC_win_printf(win, "%@%8%s%8",
            UC_XY(view->xco, view->yco - 1),
            b[BDR_RIGHT_T], view->name,
            b[BDR_LEFT_T]);
    }
}

// -----------------------------------------------------------------------

static void draw_scrollable_vertical(uC_window_t *win,
    uC_widget_view_t *view)
{
    int i;
    uC_list_node_t *n1;
    uC_widget_t *widget;

    justify = true;         // left justify button textaw

    n1 = uC_list_scan(&view->widgets, NULL);

    // scan to widget that is at the top of the view

    for (i = 0; i < view->top; i++)
    {
        n1 = uC_list_scan(NULL, n1);
    }

    for (i = 0; i < view->height && n1; i++)
    {
        widget = (uC_widget_t *)n1->payload;

        widget->focused = (i == view->cy);

        // the view controls the placement of widgets when
        // the view is scrollable.

        widget->xco = 0;
        widget->yco = 0;

        draw_widget(win, widget, view->xco, view->yco + i);

        n1 = uC_list_scan(NULL, n1);
    }

    justify = false;
}

// -----------------------------------------------------------------------

static void draw_scrollable_horizontal(uC_window_t *win,
    uC_widget_view_t *view)
{
    uint16_t i;
    uint16_t visible;
    uint16_t widget_width;
    uC_list_node_t *node;
    uC_widget_t *widget;

    node = uC_list_scan(&view->widgets, NULL);
    if (node == NULL)
    {
        return;
    }

    widget = (uC_widget_t *)node->payload;
    widget_width = widget->width;
    if (widget_width == 0)
    {
        return;
    }

    visible = view->width / widget_width;

    for (i = 0; (i < view->top) && node; i++)
    {
        node = uC_list_scan(NULL, node);
    }

    win->attrs = view->attrs;
    widget_clear_width(win, view->xco, view->yco, view->width);

    justify = false;
    for (i = 0; (i < visible) && node; i++)
    {
        widget = (uC_widget_t *)node->payload;
        widget->focused = (i == view->cy);

        // Horizontal views own widget placement just as vertical views do.

        widget->xco = 0;
        widget->yco = 0;

        draw_widget(win, widget, view->xco + (i * widget_width),
            view->yco);

        node = uC_list_scan(NULL, node);
    }
}

// -----------------------------------------------------------------------

static void draw_scrollable(uC_window_t *win, uC_widget_view_t *view)
{
    if (view->orientation == uC_VIEW_HORIZONTAL)
    {
        draw_scrollable_horizontal(win, view);
    }
    else
    {
        draw_scrollable_vertical(win, view);
    }
}

// -----------------------------------------------------------------------

static void draw_nonscrollable(uC_window_t *win, uC_widget_view_t *view)
{
    uC_list_node_t *n1;

    n1 = uC_list_scan(&view->widgets, NULL);

    while (n1)
    {
        draw_widget(win, (uC_widget_t *)n1->payload,
            view->xco, view->yco);

        n1 = uC_list_scan(NULL, n1);
    }
}

// -----------------------------------------------------------------------

static void draw_view(uC_window_t *win, uC_widget_view_t *view)
{
    win->attrs = view->attrs;

    if (view->flags & (1 << uC_VIEW_BOXED))
    {
        draw_view_box(win, view);
    }

    (view->flags & (1 << uC_VIEW_SCROLL))
        ? draw_scrollable(win, view)
        : draw_nonscrollable(win, view);
}

// -----------------------------------------------------------------------

static void draw_views(uC_widget_vg_t *vg)
{
    uC_list_node_t *n1;
    uC_widget_view_t *view;

    n1 = uC_list_scan(&vg->views, NULL);

    while (n1)
    {
        view = (uC_widget_view_t *)n1->payload;

        draw_view(&vg->window, view);

        n1 = uC_list_scan(NULL, n1);
    }
}

// -----------------------------------------------------------------------

void draw_widget_view_group(uC_widget_vg_t *vg)
{
    if (vg == NULL)
    {
        return;
    }

    draw_views(vg);
}

// -----------------------------------------------------------------------

void draw_popup_view_group(uC_widget_vg_t *vg)
{
    if ((vg == NULL) || (vg->window.screen == NULL))
    {
        return;
    }

    draw_widget_view_group(vg);
    scr_draw_win(&vg->window);
}

// -----------------------------------------------------------------------

void draw_view_groups(uC_screen_t *scr)
{
    uC_list_node_t *n1;
    uC_widget_vg_t *vg;

    n1 = uC_list_scan(&scr->view_groups, NULL);

    while (n1)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        draw_widget_view_group(vg);
        scr_draw_win(&vg->window);

        n1 = uC_list_scan(NULL, n1);
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
