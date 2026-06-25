// uC_menu_shortcut.c
// -----------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "uC_menus.h"

// -----------------------------------------------------------------------

#ifdef UC_MENUS

// -----------------------------------------------------------------------

static uint8_t shortcut_key(uC_shortcut_t shortcut)
{
    return (uint8_t)(shortcut & UC_SHORTCUT_KEY_MASK);
}

// -----------------------------------------------------------------------

static uint8_t shortcut_upper(uint8_t key)
{
    if ((key >= 'a') && (key <= 'z'))
    {
        return (uint8_t)(key - ('a' - 'A'));
    }
    return key;
}

// -----------------------------------------------------------------------

static const char *shortcut_special_name(uint8_t key)
{
    switch (key)
    {
        case UC_KEY_TAB:     return "Tab";
        case UC_KEY_ENTER:   return "Enter";
        case UC_KEY_ESC:     return "Esc";
        case UC_KEY_UP:      return "Up";
        case UC_KEY_DOWN:    return "Down";
        case UC_KEY_LEFT:    return "Left";
        case UC_KEY_RIGHT:   return "Right";
        case UC_KEY_INSERT:  return "Ins";
        case UC_KEY_DELETE:  return "Del";
        case UC_KEY_HOME:    return "Home";
        case UC_KEY_END:     return "End";
        case UC_KEY_MOUSE:   return "Mouse";
        case UC_KEY_F10:     return "F10";
        case UC_KEY_BACKTAB: return "BTab";
        case UC_KEY_PGDN:    return "PgDn";
        case UC_KEY_PGUP:    return "PgUp";
        default:             return NULL;
    }
}

// -----------------------------------------------------------------------

static bool shortcut_base_display(uint8_t key, char *dst, uint16_t dst_len)
{
    const char *name;

    if ((dst == NULL) || (dst_len == 0))
    {
        return false;
    }

    if ((key >= 0x20) && (key <= 0x7e))
    {
        dst[0] = (char)shortcut_upper(key);
        if (dst_len > 1)
        {
            dst[1] = '\0';
        }
        return dst_len > 1;
    }

    name = shortcut_special_name(key);
    if (name != NULL)
    {
        snprintf(dst, dst_len, "%s", name);
        return strlen(name) < dst_len;
    }

    return false;
}

// -----------------------------------------------------------------------

bool uC_menu_shortcut_display(uC_shortcut_t shortcut, char *dst,
    uint16_t dst_len)
{
    uint8_t key = shortcut_key(shortcut);
    char base[8];
    bool alt = (shortcut & UC_SHORTCUT_MOD_ALT) != 0;
    bool ctrl = (shortcut & UC_SHORTCUT_MOD_CTRL) != 0;
    bool meta = (shortcut & UC_SHORTCUT_MOD_META) != 0;

    if ((shortcut == 0) || (key == 0) || (dst == NULL) || (dst_len == 0))
    {
        return false;
    }

    if (!shortcut_base_display(key, base, sizeof(base)))
    {
        return false;
    }

    dst[0] = '\0';

    if (meta)
    {
        snprintf(dst + strlen(dst), dst_len - strlen(dst), "M-");
    }
    if (alt)
    {
        snprintf(dst + strlen(dst), dst_len - strlen(dst), "A-");
    }
    if (ctrl)
    {
        snprintf(dst + strlen(dst), dst_len - strlen(dst), "^");
    }
    snprintf(dst + strlen(dst), dst_len - strlen(dst), "%s", base);

    return strlen(dst) < dst_len;
}

// -----------------------------------------------------------------------

uint16_t uC_menu_shortcut_width(uC_shortcut_t shortcut)
{
    char text[12];

    if (!uC_menu_shortcut_display(shortcut, text, sizeof(text)))
    {
        return 0;
    }

    return (uint16_t)strlen(text);
}

// -----------------------------------------------------------------------

bool uC_menu_shortcut_matches(uC_shortcut_t shortcut, uint8_t key)
{
    return uC_shortcut_matches(shortcut, key);
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
