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
#include "uC_mouse.h"

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;
extern void (*k_table[])(void);

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

static void k_bs(void)  { uC_set_key(0x08); }
static void k_ent(void) { uC_set_key(0x0a); }
static void k_cbt(void) { uC_set_key(0x88); }

// -----------------------------------------------------------------------
// Default key handler table.  The system calls entries by key_index_t
// ordinal when uC_key() matches an escape sequence.  Order is fixed and
// must match key_index_t exactly.  Cursor keys default to uC_noop so
// uC_key() returns 0 for them unless the app installs custom handlers
// via uC_alloc_kh() + uC_set_key_action().

static uC_key_handler_t *default_key_actions[] =
{
//  ENTER    UP       DOWN     LEFT     RIGHT    BS       DEL
    k_ent,   uC_noop, uC_noop, uC_noop, uC_noop, k_bs,    uC_noop,
//  INSERT   HOME     END      PDN      PUP      BACKTAB
    uC_noop, uC_noop, uC_noop, uC_noop, uC_noop, k_cbt,
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
// keep processing key sequences till we get a normal key

API uint8_t uC_key(void)
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
        else if (uC_mouse_parse())
        {
            break;
        }
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

// =======================================================================
