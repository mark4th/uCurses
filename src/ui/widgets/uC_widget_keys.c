// uC_widget_keys.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_utils.h"
#include "uC_terminfo.h"
#include "uC_screen.h"
#include "uC_alloc.h"
#include "uC_widgets.h"
#include "uC_list.h"
#include "uC_window.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern uC_kh_t user_key_actions;
extern uC_screen_t *active_screen;

uC_kh_t app_key_actions;
widget_state_t widget_state;

// -----------------------------------------------------------------------
// scan all widgets within view for a widget whith given sequence number

static bool scan_view(uC_widget_view_t *view, uint16_t sequence)
{
    uC_list_node_t *n1;
    uC_widget_t *widget;

    n1 = uC_list_scan(&view->widgets, NULL);

    while (n1 != NULL)
    {
        widget = (uC_widget_t *)n1->payload;
        widget->focused = false;

        if (widget->sequence == sequence)
        {
            widget->focused = true;
            widget_state.widget = widget;

            if (view->view_node == NULL)
            {
                view->view_node = n1;
            }

            return true;
        }

        n1 = uC_list_scan(NULL, n1);
    }

    return false;
}

// -----------------------------------------------------------------------
// scan all views within view group for widget with given sequence number

static bool scan_vg(uC_widget_vg_t *vg, uint16_t sequence)
{
    bool f;

    uC_list_node_t *n1;
    uC_widget_view_t *view;

    n1 = uC_list_scan(&vg->views, NULL);

    while (n1 != NULL)
    {
        view = (uC_widget_view_t *)n1->payload;

        f = scan_view(view, sequence);
        if (f == true)
        {
            widget_state.view = view;
            return true;
        }

        n1 = uC_list_scan(NULL, n1);
    }
    return false;
}

// -----------------------------------------------------------------------
// scan all view groups for widget with sequence number

static bool find_widget(uint16_t sequence)
{
    bool f = false;
    bool result;

    uC_list_node_t *n1;
    uC_widget_vg_t *vg;

    n1 = uC_list_scan(&active_screen->view_groups, NULL);

    while (n1 != NULL)
    {
        vg = (uC_widget_vg_t *)n1->payload;

        if (f == false)
        {
            vg->window.flags &= ~WIN_FOCUS;

            f = scan_vg(vg, sequence);

            if (f == true)
            {
                result = f;
                vg->window.flags |= WIN_FOCUS;
                widget_state.vg = vg;
                widget_state.sequence = sequence;
            }
        }
        n1 = uC_list_scan(NULL, n1);
    }

    uC_scr_draw_screen(active_screen);

    return result;
}

// -----------------------------------------------------------------------

static bool tab_next_widget(void)
{
    bool f;

    uint16_t sequence = widget_state.sequence + 1;
    widget_state.sequence = 0;

    f = find_widget(sequence);

    if (f == false)
    {
        f = find_widget(0);
    }

    return f;
}

// -----------------------------------------------------------------------

static uint8_t handle_widget(uint8_t k)
{
    if (widget_state.widget == NULL)
    {
        return k;
    }

    if (widget_state.view != NULL)
    {
        if (((k == WIDGET_KEY_UP) || (k == WIDGET_KEY_DOWN)) &&
            (widget_state.view->flags & (1 << VIEW_SCROLL)))
        {
            widget_scroll_view(k);
            return k;
        }
    }

    switch (widget_state.widget->type)
    {
        case uC_WIDGET_NONE:    return k;
        case uC_WIDGET_BUTTON:  return handle_button(k);
        case uC_WIDGET_RADIO:   return handle_radio(k);
        case uC_WIDGET_CHECK:   return handle_check(k);
        case uC_WIDGET_TEXTBOX: return handle_text(k);
    }

    return k;
}

// -----------------------------------------------------------------------

static uint8_t widget_key(void)
{
    uint8_t k = 0;
    bool f;

    while (uC_test_keys() != 0)
    {
        k = uC_key();

        if ((k == 0x09) && (active_screen->view_groups.count != 0))
        {
            f = tab_next_widget();
            if (f == false)
            {
                k = 0x1b;
            }
        }

        if (k != 0x1b)
        {
            k = handle_widget(k);
        }
    }

    return k;
}

// -----------------------------------------------------------------------
// any key that returns an escape sequence instead of a single character
// is processed differently by the uCurses keyboard handler.   each key
// that is currently recognized by uCurses (and not all are) will have a
// user definable handler which will be called for that key press.  The
// handlers for the keys needed by the widget loop are given below.
//
// Each of these functions will inject a single value back into the
// uCurses keyboard input buffer to be handled as a single key press.
// this injected value will be returned to widget_key() above.
// The enumeration (at the top of this file) contains values that cannot
// be returned by a normal keypress so are safe to use

// stuff one of the above uCurses black magic key values into the keyboard
// input buffers to be returned to widget_key() above

static void widget_key_up(void)     { uC_set_key(WIDGET_KEY_UP);     }
static void widget_key_down(void)   { uC_set_key(WIDGET_KEY_DOWN);   }
static void widget_key_left(void)   { uC_set_key(WIDGET_KEY_LEFT);   }
static void widget_key_right(void)  { uC_set_key(WIDGET_KEY_RIGHT);  }
static void widget_key_insert(void) { uC_set_key(WIDGET_KEY_INSERT); }
static void widget_key_delete(void) { uC_set_key(WIDGET_KEY_DELETE); }
static void widget_key_home(void)   { uC_set_key(WIDGET_KEY_HOME);   }
static void widget_key_end(void)    { uC_set_key(WIDGET_KEY_END);    }

// -----------------------------------------------------------------------
// the other option is to simply have each of these perform the desired
// action but then each and every single one of these would need to base
// that action on which widget type is currently active.   The code to
// arbitrate this already exists above so we only need to implement it one
// time this way.

static void set_widget_key_actions(void)
{
    uC_alloc_kh();

    uC_set_key_action(K_CUU1, widget_key_up);
    uC_set_key_action(K_CUD1, widget_key_down);
    uC_set_key_action(K_CUB1, widget_key_left);
    uC_set_key_action(K_CUF1, widget_key_right);
    uC_set_key_action(K_ICH1, widget_key_insert);
    uC_set_key_action(K_DCH1, widget_key_delete);
    uC_set_key_action(K_HOME, widget_key_home);
    uC_set_key_action(K_END,  widget_key_end);
}

// -----------------------------------------------------------------------

API uint8_t uC_widget_main(void)
{
    uint8_t k;

    app_key_actions = user_key_actions;

    set_widget_key_actions();

    k = widget_key();

    uC_free(uC_MEM_ZONE_UI, user_key_actions);

    user_key_actions = app_key_actions;

    return k;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
