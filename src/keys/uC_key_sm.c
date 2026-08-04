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
// CSI: ESC [ <params> <final>.  params are decimal numbers separated by ';';
// param[0] selects "~" keys, param[1] (when present) is the modifier code
// (bitmask = value - 1: bit0 shift, bit1 alt, bit2 ctrl).  Returns the
// key_index_t or SM_UNHANDLED (unknown final, or a mouse introducer left for
// the caller's mouse parser).

static int16_t decode_csi(void)
{
    int32_t param[2] = { 0, 0 };
    uint8_t np = 0;
    bool have_digit = false;
    uint8_t i = 2;                      // keybuff[0]=ESC, [1]='['

    // ESC [ M  (X10 mouse) and ESC [ <  (SGR mouse) are not keys
    if ((ti_vars->num_k > 2) &&
        ((ti_vars->keybuff[2] == 'M') || (ti_vars->keybuff[2] == '<')))
    {
        return SM_UNHANDLED;
    }

    while (i < ti_vars->num_k)
    {
        uint8_t c = ti_vars->keybuff[i++];

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
            continue;
        }
        if (c == ';')
        {
            if (np < 2) { np++; }
            have_digit = false;
            continue;
        }
        // any other byte is the final byte of the sequence
        if (have_digit || (np > 0)) { np++; }   // count the trailing param

        key_mods = (np >= 2) ? (uint8_t)((param[1] - 1) & 0x07) : 0;

        if (c == '~')
        {
            return tilde_number(param[0]);
        }
        return final_letter(c, key_mods);
    }

    return SM_UNHANDLED;                 // ran out of bytes: incomplete
}

// -----------------------------------------------------------------------
// decode the ESC-initiated sequence in keybuff.  keybuff[0] is guaranteed
// 0x1b by the caller.

int16_t sm_parse(void)
{
    key_mods = 0;

    // bare ESC — nothing followed it within the poll window
    if (ti_vars->num_k == 1)
    {
        return SM_DIRECT;               // keybuff[0] already 0x1b
    }

    switch (ti_vars->keybuff[1])
    {
        case '[':                       // CSI
            return decode_csi();

        case 'O':                       // SS3
            return (ti_vars->num_k > 2)
                ? decode_ss3(ti_vars->keybuff[2])
                : SM_DIRECT;            // lone ESC O — treat ESC as bare

        case 0x1b:                      // ESC ESC -> report a bare ESC
            ti_vars->num_k = 1;
            return SM_DIRECT;

        default:                        // ESC <printable> = Alt+char
            if (ti_vars->num_k == 2)
            {
                key_mods         = KMOD_ALT;
                ti_vars->keybuff[0] = ti_vars->keybuff[1];
                ti_vars->num_k      = 1;
                return SM_DIRECT;
            }
            return SM_UNHANDLED;
    }
}

// =======================================================================
