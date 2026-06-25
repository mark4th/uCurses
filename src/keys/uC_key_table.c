// uC_key_table.c   - terminal keyboard handler
// -----------------------------------------------------------------------

#include <stdint.h>
#include <string.h>

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_utils.h"
#include "uC_parse.h"
#include "uC_keys.h"
#include "uC_alloc.h"
#include "uC_terminfo.h"
#include "uC_screen.h"
#ifdef UC_MENUS
#include "uC_menus.h"
#endif
#ifdef UC_WIDGETS
#include "uC_widgets.h"
#endif
#ifdef UC_MOUSE
#include "uC_mouse.h"
#endif

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;
extern void (*k_table[])(void);
extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------

API void uC_set_key(uint8_t c)
{
    ti_vars->keybuff[0] = c;
    ti_vars->num_k      = 1;
    ti_vars->stuffed    = true;
}

// -----------------------------------------------------------------------

API void uC_flush_keys(void)
{
    ti_vars->num_k = 0;
}

// -----------------------------------------------------------------------
// add backspace, add enter key to keyboard input buffer

static void k_bs(void)      { uC_set_key(UC_KEY_BS);      }
static void k_ent(void)     { uC_set_key(UC_KEY_ENTER);   }
static void k_up(void)      { uC_set_key(UC_KEY_UP);      }
static void k_down(void)    { uC_set_key(UC_KEY_DOWN);    }
static void k_left(void)    { uC_set_key(UC_KEY_LEFT);    }
static void k_right(void)   { uC_set_key(UC_KEY_RIGHT);   }
static void k_delete(void)  { uC_set_key(UC_KEY_DELETE);  }
static void k_insert(void)  { uC_set_key(UC_KEY_INSERT);  }
static void k_home(void)    { uC_set_key(UC_KEY_HOME);    }
static void k_end(void)     { uC_set_key(UC_KEY_END);     }
static void k_pgdn(void)    { uC_set_key(UC_KEY_PGDN);    }
static void k_pgup(void)    { uC_set_key(UC_KEY_PGUP);    }
static void k_cbt(void)     { uC_set_key(UC_KEY_BACKTAB); }

// -----------------------------------------------------------------------
// Default key handler table.  The system calls entries by key_index_t
// ordinal when uC_key() matches an escape sequence.  Order is fixed and
// must match key_index_t exactly.  Common navigation/editing keys default
// to UC_KEY_* values, and features can opt in extra keys before apps
// override individual handlers via uC_alloc_kh() + uC_set_key_action().

static uC_key_handler_t *default_key_actions[] =
{
//  ENTER    UP       DOWN     LEFT     RIGHT    BS       DEL
    k_ent,   k_up,    k_down,  k_left,  k_right, k_bs,    k_delete,
//  INSERT   HOME     END      PDN      PUP      BACKTAB
    k_insert, k_home, k_end,   k_pgdn,  k_pgup,  k_cbt,
//  F1       F2       F3       F4       F5       F6
    uC_noop, uC_noop, uC_noop, uC_noop, uC_noop, uC_noop,
//  F7       F8       F9       F10      F11      F12
    uC_noop, uC_noop, uC_noop, uC_noop, uC_noop, uC_noop
};

#define KEY_COUNT (sizeof(default_key_actions) / sizeof(default_key_actions[0]))

// -----------------------------------------------------------------------

static uC_kh_t user_key_actions = default_key_actions;

// -----------------------------------------------------------------------

API uC_kh_t uC_alloc_kh(void)
{
    uC_kh_t kh;
    uC_kh_t save = NULL;

    kh = uC_alloc(uC_MEM_ZONE_UI, sizeof(default_key_actions));

    if (kh != NULL)
    {
        save = user_key_actions;
        memcpy(kh, default_key_actions, sizeof(default_key_actions));
        user_key_actions = kh;
    }

    return save;
}

// -----------------------------------------------------------------------

API void uC_release_kh(uC_kh_t saved)
{
    uC_ui_free(user_key_actions);
    user_key_actions = saved;
}

// -----------------------------------------------------------------------

API uC_key_handler_t *uC_set_key_action(key_index_t index,
    uC_key_handler_t *action)
{
    uC_key_handler_t *x = (user_key_actions)[index];
    user_key_actions[index] = action;

    return x;
}

// -----------------------------------------------------------------------

uC_key_handler_t *uC_set_default_key_action(key_index_t index,
    uC_key_handler_t *action)
{
    uC_key_handler_t *x = default_key_actions[index];
    default_key_actions[index] = action;

    return x;
}

// -----------------------------------------------------------------------

bool uC_restore_default_key_action(key_index_t index,
    uC_key_handler_t *expected, uC_key_handler_t *action)
{
    if (default_key_actions[index] != expected)
    {
        return false;
    }

    default_key_actions[index] = action;
    return true;
}

// -----------------------------------------------------------------------

bool uC_restore_key_action(key_index_t index,
    uC_key_handler_t *expected, uC_key_handler_t *action)
{
    if (user_key_actions[index] != expected)
    {
        return false;
    }

    user_key_actions[index] = action;
    return true;
}

// -----------------------------------------------------------------------
// keep processing key sequences till we get a normal key

uint8_t uC_key_raw(void)
{
    int16_t c;

    while (ti_vars->num_k != 1)
    {
        memset(ti_vars->keybuff, 0, KEY_BUFF_SZ);
        uC_read_keys();     // read key or sequence into keyboard buff

        // if the first byte is a regular character (not 0x1b) then we
        // have a normal keypress that may have arrived alongside mouse
        // event bytes (all-motion mouse generates bytes continuously).
        // truncate to one byte and return it immediately rather than
        // looping, which would memset the buffer and lose the keypress.

        if (ti_vars->keybuff[0] != 0x1b)
        {
            ti_vars->num_k = 1;
            break;
        }

        c = match_key();    // compare input with all handled escapes

        if (c != -1)        // if escape sequence is one we handle
        {                   // internally
            // flush the escape buffer
            ti_vars->num_esc = 0;

            // execute handler for keypress
            user_key_actions[c]();

            // the above call to the user_key_actions() function
            // can return a custom key press value for any of
            // the key sequence keys.

            if (ti_vars->num_k == 1)
            {
                break;
            }
            ti_vars->num_k = 0;
            return 0;
        }
#ifdef UC_MOUSE
        else if (uC_mouse_parse())
        {
            break;
        }
#endif
    }

    // no matter what value your terminal returns for a press of the
    // backspace key, the following code will ensure that *only* a value
    // of 0x08 needs to be interpreted elsewhere within this library.

    if ((ti_vars->num_k == 1) && (ti_vars->keybuff[0] == 0x7f))
    {
        ti_vars->keybuff[0] = 0x08;
    }

    ti_vars->num_k = 0;

    return ti_vars->keybuff[0];
}

// -----------------------------------------------------------------------

API uint8_t uC_key(void)
{
    uint8_t key;
    uint8_t out = UC_KEY_NONE;

    key = uC_key_raw();

    if (uC_scr_shortcuts_enabled(active_screen) &&
#ifdef UC_WIDGETS
        !widget_text_input_active(active_screen) &&
#endif
        uC_shortcut_run(active_screen, key))
    {
        out = uC_key_raw();
        return (out == 0xff) ? UC_KEY_NONE : out;
    }

#ifdef UC_MENUS
    if (menu_key(active_screen, key, &out))
    {
        return out;
    }
#endif

#ifdef UC_WIDGETS
    if (widget_key(active_screen, key, &out))
    {
        return out;
    }
#endif

    return key;
}

// =======================================================================
