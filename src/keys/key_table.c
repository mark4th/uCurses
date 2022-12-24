// keys.c   - terminal keyboard handler
// -----------------------------------------------------------------------

#include <stdint.h>
#include <string.h>

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_utils.h"
#include "uC_parse.h"
#include "uC_keys.h"

extern ti_parse_t *uC_ti_parse;
extern int8_t keybuff[KEY_BUFF_SZ];
extern int16_t num_k;
extern int8_t stuffed;
extern void (*k_table[])(void);

// -----------------------------------------------------------------------

static void set_kb0(int8_t c)
{
    keybuff[0] = c;
    num_k = 1;
}

// -----------------------------------------------------------------------
// add delete char to keyboard input buffer

static void k_bs(void)  { set_kb0(0x08); }
static void k_ent(void) { set_kb0(0x0a); }

// -----------------------------------------------------------------------
// the system indirectly calls the function pointers pointed to by the
// items in this array.  the end user does not get to modify this array as
// the order of items within it is critical

static key_handler_t *default_key_actions[] =
{
//  ENTER  UP     DOWN  LEFT  RIGHT BS    BS2
    k_ent, uC_noop,  uC_noop, uC_noop, uC_noop, k_bs, k_bs,
//  DEL    INSERT HOME  END   PDN   PUP   F1
    uC_noop,  uC_noop,  uC_noop, uC_noop, uC_noop, uC_noop, uC_noop,
//  F2     F3     F4    F5    F6    F7    F8
    uC_noop,  uC_noop,  uC_noop, uC_noop, uC_noop, uC_noop, uC_noop,
//  F9     F10    F11   F12
    uC_noop,  uC_noop,  uC_noop, uC_noop
};

#define KEY_COUNT (sizeof(default_key_actions) / sizeof(default_key_actions[0]))

// -----------------------------------------------------------------------

static key_handler_t *user_key_actions[KEY_COUNT] = { NULL };

// -----------------------------------------------------------------------

API key_handler_t *uC_set_key_action(key_index_t index, key_handler_t *action)
{
    key_handler_t *x = user_key_actions[index];
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
        read_keys();        // read key or sequence into keyboard buff

        c = match_key();    // compare input with all handled escapes

        if (c != -1)        // if escape sequence is one we handle
        {                   // internally
            // flush the escape buffer
            uC_ti_parse->num_esc = 0;

            // execute handler for keypress
            user_key_actions[c]();

            // backspace or enter
            if (num_k == 1) { break; }
            num_k = 0;
            return 0;
        }
    }

    num_k = 0;
    return keybuff[0];
}

// -----------------------------------------------------------------------
// manually stuff a key into the buffer as if a key had been pressed.

API void uC_stuff_key(int8_t c)
{
    stuffed    = 1;         // does not stuff a key sequene, just one key
    num_k      = 1;
    keybuff[0] = c;
}

// -----------------------------------------------------------------------

void init_key_handlers(void)
{
    memcpy(user_key_actions, default_key_actions,
        sizeof(user_key_actions));
}

// =======================================================================
