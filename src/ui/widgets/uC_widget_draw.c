// uC_widget_draw.c
// -----------------------------------------------------------------------

#include <string.h>

#include "uC_widgets.h"
#include "uC_window.h"
#include "uC_utils.h"
#include "uC_list.h"
#include "uC_win_printf.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;
extern border_t *const borders[];

// -----------------------------------------------------------------------
// for radio button and check box widgets

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

static void draw_widget(uC_window_t *win, uC_widget_t *widget,
    uint16_t xco, uint16_t yco)
{
    uint16_t x = (xco + widget->xco);
    uint16_t y = (yco + widget->yco);

    // widget coordinates are relative to the view position within its
    // parent window. position the cursor to the widgets location.

    // %@ set cursor x / y location within window

    uC_win_printf(win, "%@", x, y);

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
// clear a specified rectangular area within a window

static void clear_box(uC_window_t *win, uint16_t xco, uint16_t yco,
    uint16_t width, uint16_t height)
{
    while (height--)
    {
        // %@ set cursor x / y location within window
        // %* print multiple repetitions of char

        uC_win_printf(win, "%@%*", xco, yco++, width, 0x20);
    }
}

// -----------------------------------------------------------------------

static void draw_view_box(uC_window_t *win, uC_widget_view_t *view)
{
    border_t *b;

    win_draw_box(win, view->xco - 1, view->yco - 1,
        view->width, view->height,
        view->box_type, view->box_attrs);

    if (view->name != NULL)
    {
        b = borders[view->box_type];

        // %@ set cursor x / y location within window
        // %8 emit single utf8 character
        // %s write a string
        // %8 emit single utf8 character

        uC_win_printf(win, "%@%8%s%8",
            view->xco, view->yco - 1,
            b[BDR_RIGHT_T], view->name,
            b[BDR_LEFT_T]);
    }
}

// -----------------------------------------------------------------------

static void draw_scrollable(uC_window_t *win, uC_widget_view_t *view)
{
    int i;
    uC_list_node_t *n1;
    uC_widget_t *widget;

    n1 = uC_list_scan(&view->widgets, NULL);

    for (i = 0; i < view->top; i++)
    {
        n1 = uC_list_scan(NULL, n1);
    }

    for (i = 0; i < view->height && (n1 != NULL); i++)
    {
        widget = (uC_widget_t *)n1->payload;

        widget->focused = ((i == view->cy) &&
            (widget_state.sequence == view->sequence));

        // the view controls the placement of widgets when
        // the view is scrollable.

        widget->xco = 0;
        widget->yco = 0;

        draw_widget(win, widget, view->xco, view->yco + i);

        n1 = uC_list_scan(NULL, n1);
    }
}

// -----------------------------------------------------------------------

static void draw_nonscrollable(uC_window_t *win, uC_widget_view_t *view)
{
    uC_list_node_t *n1;

    n1 = uC_list_scan(&view->widgets, NULL);

    while (n1 != NULL)
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

    clear_box(win, view->xco, view->yco,
        view->width, view->height);

    if (view->flags & (1 << VIEW_BOXED))
    {
        draw_view_box(win, view);
    }

    (view->flags & (1 << VIEW_SCROLL))
        ? draw_scrollable(win, view)
        : draw_nonscrollable(win, view);
}

// -----------------------------------------------------------------------

static void draw_views(uC_widget_vg_t *vg)
{
    uC_list_node_t *n1;
    uC_widget_view_t *view;

    n1 = uC_list_scan(&vg->views, NULL);

    while (n1 != NULL)
    {
        view = (uC_widget_view_t *)n1->payload;

        draw_view(&vg->window, view);

        n1 = uC_list_scan(NULL, n1);
    }
}

// -----------------------------------------------------------------------

void draw_view_groups(uC_screen_t *scr)
{
    uC_list_node_t *n1;
    uC_widget_vg_t *vg;

    n1 = uC_list_scan(&scr->view_groups, NULL);

    while (n1 != NULL)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        draw_views(vg);
        scr_draw_win(&vg->window);

        n1 = uC_list_scan(NULL, n1);
    }
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
