// uC_widget_keys.c
// -----------------------------------------------------------------------

#include <string.h>

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_terminfo.h"
#include "uC_screen.h"
#include "uC_alloc.h"
#include "uC_widgets.h"
#include "uC_window.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern uC_kh_t user_key_actions;

uC_kh_t saved_key_actions;
API widget_state_t widget_state;

void uC_widget_reset_state(void)
{
    memset(&widget_state, 0, sizeof(widget_state));
    uC_widget_reset_sequence();
}

// ----------------------------------------------------------------
// if current view is scrollable and k is a cursor up or dowwn...

static bool check_scrollable(uint8_t k)
{
    // view is a known non null here

    if (widget_state.view->flags & (1 << uC_VIEW_SCROLL))
    {
        if ((k == WIDGET_KEY_UP) || (k == WIDGET_KEY_DOWN))
        {
            widget_scroll_view(k);
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------------------

static uint8_t handle_widget_key(uint8_t k)
{
    bool f;

    if (!widget_state.widget)
    {
        return k;
    }

    // handle case where the current view is scrollable and
    // either cursor up or cursor down have been pressed

    if (widget_state.view)
    {
        f = check_scrollable(k);
        if (f)
        {
            return k;
        }
    }

    // other than cursor up / down within a scrollable view (wich can
    // contain any widget type) each widget type module contains its
    // own keyboard handler

    switch (widget_state.widget->type)
    {
        case uC_WIDGET_NONE:    return k;
        case uC_WIDGET_BUTTON:  return handle_button(k);
        case uC_WIDGET_RADIO:   return handle_radio(k);
        case uC_WIDGET_CHECK:   return handle_check(k);
        case uC_WIDGET_TEXTBOX: return handle_textbox(k);
    }

    return k;
}

// -----------------------------------------------------------------------

static uint8_t _widget_key(void)
{
    uint8_t k;

    k = uC_key_raw();    // blocks till a key is pressed

    switch (k)
    {
        case UC_KEY_TAB:       tab_next_widget();        break;
        case UC_KEY_BACKTAB:   tab_prev_widget();        break;
#ifdef UC_MOUSE
        case WIDGET_KEY_MOUSE: k = uC_widget_mouse_handle(); break;
#endif

        default:
            k = handle_widget_key(k);
    }

    return k;
}

// -----------------------------------------------------------------------

static uint8_t widget_read_key(void)
{
    uint8_t k = 0;

    // this while loop ensures that when an escape sequence key
    // is pressed and the uCurses keyboard handler translates that
    // into a single character we will read and interpret that single
    // character within this call to widget_key().  this negates type
    // ahead

    while (uC_test_keys())
    {
        k = _widget_key();
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
// The enumeration in uC_widgets.h contains values that cannot be returned
// by a normal keypress so are safe to use

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
static void widget_key_f10(void)    { uC_set_key(WIDGET_KEY_F10);    }

// -----------------------------------------------------------------------
// the other option is to simply have each of these perform the desired
// action but then each and every single one of these would need to base
// that action on which widget type is currently active.   The code to
// arbitrate this already exists above so we only need to implement it one
// time this way.

static void set_widget_key_actions(void)
{
    saved_key_actions = uC_alloc_kh();

    uC_set_key_action(K_CUU1, widget_key_up);
    uC_set_key_action(K_CUD1, widget_key_down);
    uC_set_key_action(K_CUB1, widget_key_left);
    uC_set_key_action(K_CUF1, widget_key_right);
    uC_set_key_action(K_ICH1, widget_key_insert);
    uC_set_key_action(K_DCH1, widget_key_delete);
    uC_set_key_action(K_HOME, widget_key_home);
    uC_set_key_action(K_END,  widget_key_end);

    // The legacy widget loop keeps F10 visible to callers. The consolidated
    // uC_key() dispatcher gives the menu system first shot at F10.

    uC_set_key_action(K_F10, widget_key_f10);
}

// -----------------------------------------------------------------------

API uint16_t uC_widget_current_sequence(void)
{
    return widget_state.sequence;
}


// -----------------------------------------------------------------------

static bool widget_input_active(uC_screen_t *scr)
{
    if ((scr == NULL) || (widget_state.screen != scr) ||
        (widget_state.vg == NULL) || (widget_state.view == NULL) ||
        (widget_state.widget == NULL))
    {
        return false;
    }

    return (widget_state.vg->flags & uC_vg_flag_inactive) == 0;
}

// -----------------------------------------------------------------------

bool widget_text_input_active(uC_screen_t *scr)
{
    return widget_input_active(scr) &&
        widget_state.widget->type == uC_WIDGET_TEXTBOX;
}

// -----------------------------------------------------------------------

bool widget_key(uC_screen_t *scr, uint8_t key, uint8_t *out)
{
    if (out != NULL)
    {
        *out = key;
    }

    if (!widget_input_active(scr))
    {
        return false;
    }

    switch (key)
    {
        case UC_KEY_TAB:
            if (out != NULL)
            {
                *out = tab_next_widget();
            }
            return true;

        case UC_KEY_BACKTAB:
            if (out != NULL)
            {
                *out = tab_prev_widget();
            }
            return true;

#ifdef UC_MOUSE
        case WIDGET_KEY_MOUSE:
            if (out != NULL)
            {
                *out = uC_widget_mouse_handle();
            }
            return true;
#endif

        default:
            if (out != NULL)
            {
                *out = handle_widget_key(key);
            }
            return true;
    }
}

// -----------------------------------------------------------------------

API char uC_widget_main(void)
{
    char k;

    if (widget_state.vg)
    {
        if (widget_state.vg->flags & uC_vg_flag_inactive)
        {
            widget_state.sequence = 0;
        }
    }

    set_widget_key_actions();

    k = widget_read_key();

    uC_release_kh(saved_key_actions);

    return k;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
