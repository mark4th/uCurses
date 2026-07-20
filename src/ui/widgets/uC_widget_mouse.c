// uC_widget_mouse.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_mouse.h"
#include "uC_widgets.h"
#include "uC_screen.h"
#include "uC_list.h"
#include "uC_keys.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------

typedef struct
{
    uC_widget_vg_t *vg;
    uC_widget_view_t *view;
    uC_widget_t *widget;
    uC_list_node_t *widget_node;
    uint16_t widget_index;
} widget_hit_t;

// -----------------------------------------------------------------------

static uint8_t widget_activate_mouse_hit(widget_hit_t *hit)
{
    if ((hit == NULL) || (hit->widget == NULL))
    {
        return UC_KEY_NONE;
    }

    if (widget_state.widget != NULL)
    {
        widget_state.widget->focused = false;
    }
    if (widget_state.vg != NULL)
    {
        widget_state.vg->window.flags &= ~uC_WIN_FOCUS;
    }

    hit->vg->window.flags |= uC_WIN_FOCUS;
    hit->widget->focused = true;
    hit->view->view_node = hit->widget_node;

    if (hit->view->flags & (1 << uC_VIEW_SCROLL))
    {
        if (hit->view->orientation == uC_VIEW_HORIZONTAL)
        {
            hit->view->cy = hit->widget_index - hit->view->top;
        }
        else
        {
            hit->view->cy = hit->widget_index - hit->view->top;
        }
    }

    widget_state.vg = hit->vg;
    widget_state.view = hit->view;
    widget_state.widget = hit->widget;
    widget_state.sequence = hit->widget->sequence;

    switch (hit->widget->type)
    {
        case uC_WIDGET_BUTTON:   return handle_button(UC_KEY_ENTER);
        case uC_WIDGET_RADIO:    return handle_radio(0x20);
        case uC_WIDGET_CHECK:    return handle_check(0x20);

        case uC_WIDGET_TEXTBOX:
        case uC_WIDGET_NONE:
            break;
    }

    return UC_KEY_NONE;
}

// -----------------------------------------------------------------------
// walk vg → views → widgets to find which widget (if any) contains (sx,sy)
// returns true and fills hit when a widget is under the mouse pointer

static bool widget_hit_test(int16_t sx, int16_t sy, widget_hit_t *hit)
{
    uC_list_node_t   *n1, *n2, *n3;
    uC_widget_vg_t   *vg;
    uC_widget_view_t *view;
    uC_widget_t      *widget;

    int16_t vg_inner_x;
    int16_t vg_inner_y;
    int16_t view_inner_x;
    int16_t view_inner_y;
    int16_t wx, wy;
    uint16_t index;
    uint16_t visible;
    uint16_t widget_width;
    bool widget_visible;

    if (hit == NULL)
    {
        return false;
    }

    *hit = (widget_hit_t){ 0 };

    n1 = uC_list_scan(&widget_state.screen->view_groups, NULL);

    while (n1)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        if (vg->flags & uC_vg_flag_inactive)
        {
            n1 = uC_list_scan(NULL, n1);
            continue;
        }

        vg_inner_x = vg->window.xco +
            ((vg->window.flags & uC_WIN_BOXED) ? 1 : 0);
        vg_inner_y = vg->window.yco +
            ((vg->window.flags & uC_WIN_BOXED) ? 1 : 0);

        n2 = uC_list_scan(&vg->views, NULL);

        while (n2)
        {
            view  = (uC_widget_view_t *)n2->payload;
            view_inner_x = vg_inner_x + view->xco;
            view_inner_y = vg_inner_y + view->yco;

            n3 = uC_list_scan(&view->widgets, NULL);
            index = 0;

            while (n3)
            {
                widget = (uC_widget_t *)n3->payload;
                widget_visible = false;

                if (view->flags & (1 << uC_VIEW_SCROLL))
                {
                    if (view->orientation == uC_VIEW_HORIZONTAL)
                    {
                        widget_width = widget->width;
                        visible = (widget_width != 0)
                            ? view->width / widget_width : 0;

                        if ((index >= view->top) &&
                            (index < view->top + visible))
                        {
                            wx = view_inner_x +
                                ((int16_t)(index - view->top) *
                                 (int16_t)widget_width);
                            wy = view_inner_y;
                            widget_visible = true;
                        }
                    }
                    else if ((index >= view->top) &&
                             (index < view->top + view->height))
                    {
                        wx = view_inner_x;
                        wy = view_inner_y +
                            (int16_t)(index - view->top);
                        widget_visible = true;
                    }
                }
                else
                {
                    wx = view_inner_x + (int16_t)widget->xco;
                    wy = view_inner_y + (int16_t)widget->yco;
                    widget_visible = true;
                }

                if (!widget->disabled && widget_visible)
                {
                    if (sy == wy && sx >= wx && sx < wx + (int16_t)widget->width)
                    {
                        hit->vg = vg;
                        hit->view = view;
                        hit->widget = widget;
                        hit->widget_node = n3;
                        hit->widget_index = index;
                        return true;
                    }
                }

                n3 = uC_list_scan(NULL, n3);
                index++;
            }

            n2 = uC_list_scan(NULL, n2);
        }

        n1 = uC_list_scan(NULL, n1);
    }

    return false;
}

// -----------------------------------------------------------------------
// dispatch a parsed mouse event: left-click transfers focus and activates

uint8_t uC_widget_mouse_handle(void)
{
    int16_t  sx  = uC_mouse_event.x - 1;
    int16_t  sy  = uC_mouse_event.y - 1;
    widget_hit_t hit;

    if (!widget_state.screen)
    {
        return UC_KEY_NONE;
    }

    if (uC_mouse_event.button != UC_MOUSE_LEFT || !uC_mouse_event.pressed)
    {
        return UC_KEY_NONE;
    }

    if (!widget_hit_test(sx, sy, &hit))
    {
        return UC_KEY_NONE;
    }

    return widget_activate_mouse_hit(&hit);
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
