// uC_key_sm.c   - keyboard input state machine
// -----------------------------------------------------------------------
// Decodes the ESC-initiated byte sequence buffered in ti_vars->keybuff by
// walking it one character at a time through a small state machine, instead
// of hash-matching the whole buffer against terminfo-generated sequences
// (the old match_key()).  This is what lets us report modifiers: terminfo
// has no entries for Ctrl/Shift/Alt+key, which arrive as CSI parameters
// (ESC [ 1 ; 5 A) or an ESC prefix (ESC b = Alt-b).
//
// sm_parse() is called by uC_key_raw() ONLY when keybuff[0] == 0x1b (a plain
// printable is returned before we get here).  It sets key_mods and returns:
//   >= 0          a key_index_t   -> caller runs user_key_actions[r]
//   SM_DIRECT     keycode already left in keybuff[0] (bare ESC, Alt+char)
//   SM_UNHANDLED  not a key sequence (mouse / unknown) -> caller falls through
// -----------------------------------------------------------------------

#include <stdint.h>

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_terminfo.h"

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;

// modifier mask (KMOD_*) of the most recently decoded key.  uC_alt() reads
// its Alt bit; key_mods() exposes the whole mask (Ctrl/Shift/Alt).

uint8_t key_mods;

// -----------------------------------------------------------------------
// map a CSI/SS3 final letter (A B C D H F) to its key_index_t.  shift+left
// and shift+right have dedicated table slots (K_SLFT / K_SRIT); everything
// else returns the base cursor/home/end index with mods reported separately.

static int16_t final_letter(uint8_t f, uint8_t mods)
{
    switch (f)
    {
        case 'A': return K_CUU1;                            // up
        case 'B': return K_CUD1;                            // down
        case 'C': return (mods & KMOD_SHIFT) ? K_SRIT : K_CUF1;  // right
        case 'D': return (mods & KMOD_SHIFT) ? K_SLFT : K_CUB1;  // left
        case 'H': return K_HOME;
        case 'F': return K_END;
        case 'Z': return K_BT;                              // shift-tab
        default:  return SM_UNHANDLED;
    }
}

// -----------------------------------------------------------------------
// map a CSI "~" sequence (ESC [ <n> ~) to its key_index_t by its number.

static int16_t tilde_number(int32_t n)
{
    switch (n)
    {
        case 1:  case 7:  return K_HOME;
        case 2:           return K_ICH1;     // insert
        case 3:           return K_DCH1;     // delete
        case 4:  case 8:  return K_END;
        case 5:           return K_KPP;      // page up
        case 6:           return K_KNP;      // page down
        case 15:          return K_F5;
        case 17:          return K_F6;
        case 18:          return K_F7;
        case 19:          return K_F8;
        case 20:          return K_F9;
        case 21:          return K_F10;
        case 23:          return K_F11;
        case 24:          return K_f12;
        default:          return SM_UNHANDLED;
    }
}

// -----------------------------------------------------------------------
// SS3: ESC O <final>.  application-cursor-mode arrows plus F1..F4.

static int16_t decode_ss3(uint8_t f)
{
    switch (f)
    {
        case 'P': return K_F1;
        case 'Q': return K_F2;
        case 'R': return K_F3;
        case 'S': return K_F4;
        default:  return final_letter(f, 0);   // O-prefixed cursor keys
    }
}

// -----------------------------------------------------------------------
// byte source.  the state machine pulls the bytes that follow the initial
// ESC one at a time from a source function: it returns the next raw byte, or
// -1 when none is available.  timeout_ms is the "does the sequence continue?"
// hint — the live (fd-backed) source waits that long before declaring the
// sequence finished (bare ESC / end of a CSI); the buffer source ignores it
// and just hands out the next buffered byte.  Decoupling the SM from the byte
// source this way lets it run either over a pre-filled keybuff (today) or
// straight off the tty (streaming), and lets the unit tests feed it an array.

static sm_source_t sm_src;
static void       *sm_ctx;

#define SM_INTRA_MS (25)                // per-byte wait while inside a sequence

static int sm_pull(int timeout_ms)
{
    return sm_src(sm_ctx, timeout_ms);
}

// -----------------------------------------------------------------------
// CSI: ESC [ <params> <final>.  params are decimal numbers separated by ';';
// param[0] selects "~" keys, param[1] (when present) is the modifier code
// (bitmask = value - 1: bit0 shift, bit1 alt, bit2 ctrl).  Returns the
// key_index_t or SM_UNHANDLED (unknown/incomplete final, or a mouse
// introducer left for the caller's mouse parser).

static int16_t sm_csi(void)
{
    int32_t param[2] = { 0, 0 };
    uint8_t np = 0;
    bool have_digit = false;

    int c = sm_pull(SM_INTRA_MS);       // first byte after '['

    // ESC [ M (X10 mouse) and ESC [ < (SGR mouse) are not keys.  drain the
    // rest of the report so the mouse parser (which reads keybuff/num_k)
    // sees the whole thing, then hand off.
    if ((c == 'M') || (c == '<'))
    {
        while (sm_pull(SM_INTRA_MS) >= 0)
        {
            ;
        }
        return SM_UNHANDLED;
    }

    while (c >= 0)
    {
        if ((c >= '0') && (c <= '9'))
        {
            // clamp before the multiply: real params are tiny (tilde keys
            // <= 24, modifier <= 8), so a long digit run is a malformed or
            // malicious sequence.  capping below 100000 keeps param[] well
            // clear of int32 overflow (UB) while still overshooting every
            // valid value, so tilde_number()/final_letter() reject it.
            if ((np < 2) && (param[np] < 100000))
            {
                param[np] = (param[np] * 10) + (c - '0');
            }
            have_digit = true;
        }
        else if (c == ';')
        {
            if (np < 2) { np++; }
            have_digit = false;
        }
        else                            // any other byte is the final byte
        {
            if (have_digit || (np > 0)) { np++; }   // count trailing param

            key_mods = (np >= 2) ? (uint8_t)((param[1] - 1) & 0x07) : 0;

            if (c == '~')
            {
                return tilde_number(param[0]);
            }
            return final_letter((uint8_t)c, key_mods);
        }
        c = sm_pull(SM_INTRA_MS);
    }

    return SM_UNHANDLED;                 // ran out of bytes: incomplete
}

// -----------------------------------------------------------------------
// run the state machine over `src`.  the leading ESC is already consumed by
// the caller (for the buffer source it still sits in keybuff[0]); sm_run
// pulls the bytes that follow it.  returns a key_index_t (caller runs the
// handler), SM_DIRECT (final keycode already in keybuff[0]: bare ESC /
// Alt+char), or SM_UNHANDLED (mouse / unknown -> caller falls through).

int16_t sm_run(sm_source_t src, void *ctx)
{
    sm_src   = src;
    sm_ctx   = ctx;
    key_mods = 0;

    int b = sm_pull(SM_INTRA_MS);       // first byte after ESC

    if (b < 0)                          // bare ESC — nothing followed
    {
        return SM_DIRECT;               // keybuff[0] already 0x1b
    }

    switch (b)
    {
        case '[':                       // CSI
            return sm_csi();

        case 'O':                       // SS3
        {
            int f = sm_pull(SM_INTRA_MS);
            return (f >= 0) ? decode_ss3((uint8_t)f)
                            : SM_DIRECT;        // lone ESC O — ESC as bare
        }

        case 0x1b:                      // ESC ESC -> report a bare ESC
            ti_vars->num_k = 1;
            return SM_DIRECT;

        default:                        // ESC <byte> = Alt+char, iff alone
        {
            int b2 = sm_pull(SM_INTRA_MS);
            if (b2 < 0)
            {
                key_mods            = KMOD_ALT;
                ti_vars->keybuff[0] = (uint8_t)b;
                ti_vars->num_k      = 1;
                return SM_DIRECT;
            }
            return SM_UNHANDLED;        // ESC + more bytes: not a simple Alt
        }
    }
}

// -----------------------------------------------------------------------
// buffer source: hands out the already-read bytes of keybuff one at a time,
// starting just past the leading ESC at [0].  used by sm_parse() (the
// buffered read path) and by the unit tests.  the streaming read path
// supplies an fd-backed source to sm_run() instead.

typedef struct
{
    uint8_t i;
} buf_src_t;

static int buf_source(void *ctx, int timeout_ms)
{
    buf_src_t *s = ctx;

    (void)timeout_ms;                   // buffered bytes are already here

    if (s->i >= ti_vars->num_k)
    {
        return -1;
    }
    return ti_vars->keybuff[s->i++];
}

// -----------------------------------------------------------------------
// decode the ESC-initiated sequence already buffered in keybuff.  keybuff[0]
// is guaranteed 0x1b by the caller.

int16_t sm_parse(void)
{
    buf_src_t bs = { .i = 1 };          // skip the leading ESC at [0]

    return sm_run(buf_source, &bs);
}

// =======================================================================
