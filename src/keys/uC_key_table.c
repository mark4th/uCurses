// keys.c   - terminal keyboard handler
// -----------------------------------------------------------------------

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_utils.h"
#include "uC_parse.h"
#include "uC_keys.h"

// -----------------------------------------------------------------------

extern ti_parse_t *uC_ti_parse;
extern int8_t keybuff[KEY_BUFF_SZ];
extern int16_t num_k;
extern void (*k_table[])(void);

// -----------------------------------------------------------------------
// the new widget editor extension needs to be able to redefied the
// keyboard handling without ripping the rug out from under any user
// application defined handlers

static uC_kh_t saved_key_actions;

// -----------------------------------------------------------------------

API void uC_set_key(int8_t c)
{
    keybuff[0] = c;
    num_k = 1;
}

// -----------------------------------------------------------------------
// add delete char to keyboard input buffer

static void k_bs(void)  { uC_set_key(0x08); }
static void k_ent(void) { uC_set_key(0x0a); }

// -----------------------------------------------------------------------
// the system indirectly calls the function pointers pointed to by the
// items in this array.  the end user does not get to modify this array as
// the order of items within it is critical

uC_key_handler_t *default_key_actions[] =
{
//  ENTER     UP        DOWN     LEFT     RIGHT    BS    BS2
    k_ent,    uC_noop,  uC_noop, uC_noop, uC_noop, k_bs, k_bs,
//  DEL       INSERT    HOME     END      PDN      PUP      F1
    uC_noop,  uC_noop,  uC_noop, uC_noop, uC_noop, uC_noop, uC_noop,
//  F2        F3        F4       F5       F6       F7       F8
    uC_noop,  uC_noop,  uC_noop, uC_noop, uC_noop, uC_noop, uC_noop,
//  F9        F10       F11      F12
    uC_noop,  uC_noop,  uC_noop, uC_noop
};

#define KEY_COUNT (sizeof(default_key_actions) / sizeof(default_key_actions[0]))

// -----------------------------------------------------------------------

uC_kh_t user_key_actions = default_key_actions;

// -----------------------------------------------------------------------

API uC_kh_t uC_alloc_kh(void)
{
    uC_kh_t kh = NULL;
    int size;

    if (user_key_actions != default_key_actions)
    {
        free(user_key_actions);
    }

    size = sizeof(default_key_actions);
    kh   = calloc(size, 1);

    memcpy(kh, default_key_actions, size);

    user_key_actions = kh;

    return kh;
}

// -----------------------------------------------------------------------

uC_kh_t widget_alloc_kh(void)
{
    uC_kh_t kh = NULL;
    int size;

    // dont release these user application handlers but we need to
    // override them while editing widgets

    saved_key_actions = user_key_actions;

    size = sizeof(default_key_actions);
    kh   = calloc(size, 1);

    memcpy(kh, default_key_actions, size);

    user_key_actions = kh;

    return kh;
}

// -----------------------------------------------------------------------

void widget_release_kh(void)
{
    free(user_key_actions);
    user_key_actions = saved_key_actions;
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

    while (num_k != 1)
    {
        memset(keybuff, 0, KEY_BUFF_SZ);
        uC_read_keys();     // read key or sequence into keyboard buff

        c = match_key();    // compare input with all handled escapes

        if (c != -1)        // if escape sequence is one we handle
        {                   // internally
            // flush the escape buffer
            uC_ti_parse->num_esc = 0;

            // execute handler for keypress
            user_key_actions[c]();

            // the above call to the user_key_actions() function
            // can return a custom key press value for any of
            // the key sequence keys.

            if (num_k == 1) { break; }
            num_k = 0;
            return 0;
        }
    }

    num_k = 0;
    return keybuff[0];
}

// -----------------------------------------------------------------------

void init_key_handlers(void)
{
    user_key_actions = default_key_actions;
}

// =======================================================================
