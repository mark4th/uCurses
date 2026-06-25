// uC_json_value.c   - json key value setting
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_utils.h"
#include "uC_switch.h"
#include "uC_borders.h"
#include "json.h"

// -----------------------------------------------------------------------

#ifdef UC_JSON

// -----------------------------------------------------------------------

extern json_vars_t *json_vars;
extern json_state_t *json_state;

// -----------------------------------------------------------------------
// fg, bg, gray-fg, gray-bg: all write to parent attribs via API setter.
// range: fg/bg 0-255, gray 0-23.  shared error check, then direct call.

static void value_attrib(void)
{
    json_state_t *parent = json_state->parent;
    uC_attribs_t *p      = parent->structure;
    json_type_t   ktype  = json_state->struct_type;
    int32_t       v      = json_vars->key_value;
    bool          gray   = (ktype == KEY_GRAY_FG) || (ktype == KEY_GRAY_BG);

    if ((uint32_t)v > (uint32_t)(gray ? 23 : 255))
        json_error("Attrib value out of range");

    if      (ktype == KEY_FG)      uC_set_fg(p,      v);
    else if (ktype == KEY_BG)      uC_set_bg(p,      v);
    else if (ktype == KEY_GRAY_FG) uC_set_gray_fg(p, v);
    else                           uC_set_gray_bg(p, v);
}

// -----------------------------------------------------------------------
// red, green, blue: navigate to grandparent attribs; fg/bg selected by
// parent type (STRUCT_RGB_FG vs STRUCT_RGB_BG)

static void value_rgb(void)
{
    json_state_t *parent  = json_state->parent;  // STRUCT_RGB_FG/BG
    json_state_t *gp      = parent->parent;      // STRUCT_ATTRIBS etc.
    uC_attribs_t *a       = gp->structure;
    json_type_t   ktype   = json_state->struct_type;
    bool          is_bg   = (parent->struct_type == STRUCT_RGB_BG);

    if ((json_vars->key_value & ~255) != 0)
    {
        json_error("RGB FG/BG Value out of range");
    }

    uint8_t val = (uint8_t)json_vars->key_value;

    if (is_bg)
    {
        a->flags.bits |= uC_ATTR_FLAG_RGB_BG; a->flags.bits &= ~uC_ATTR_FLAG_GRAY_BG;
        if (ktype == KEY_RED)   { a->bg_r = val; return; }
        if (ktype == KEY_GREEN) { a->bg_g = val; return; }
        a->bg_b = val;
    }
    else
    {
        a->flags.bits |= uC_ATTR_FLAG_RGB_FG; a->flags.bits &= ~uC_ATTR_FLAG_GRAY_FG;
        if (ktype == KEY_RED)   { a->fg_r = val; return; }
        if (ktype == KEY_GREEN) { a->fg_g = val; return; }
        a->fg_b = val;
    }
}

// -----------------------------------------------------------------------
// xco, yco, width, height: all write int16_t fields in parent window.
// xco/width use console_width for percentage; yco/height use console_height.
// width/height mark the value with 0x8000 when a percentage so that
// value_flag can shrink them by 2 if uC_WIN_BOXED is also set.

static void value_window_dim(void)
{
    json_state_t *parent = json_state->parent;
    uC_window_t  *win    = parent->structure;
    json_type_t   ktype  = json_state->struct_type;
    bool use_w = (ktype == KEY_XCO) || (ktype == KEY_WIDTH);

    if (json_vars->percent)
    {
        int16_t mult = use_w
            ? json_vars->console_width
            : json_vars->console_height;
        json_vars->key_value = (json_vars->key_value * mult) / 100;

        if ((ktype == KEY_WIDTH) || (ktype == KEY_HEIGHT))
            json_vars->key_value |= 0x8000;
    }

    int16_t *field = (ktype == KEY_XCO)   ? &win->xco    :
                     (ktype == KEY_YCO)   ? &win->yco    :
                     (ktype == KEY_WIDTH) ? &win->width  :
                                            &win->height;
    *field = (int16_t)json_vars->key_value;
}

// -----------------------------------------------------------------------

static void value_name(void)
{
    char *name;

    json_state_t *parent = json_state->parent;
    void *structure      = parent->structure;
    json_type_t ptype    = parent->struct_type;

    size_t len = strlen((char *)json_vars->json_token);

    if (json_vars->quoted == false)  // was token quoted?
    {
        json_error("String key values must be quoted");
    }

    // copy name token to the name buff minus the quotes
    name = (char *)uC_alloc(uC_MEM_ZONE_UI, len + 1);
    if (!name)
    {
        return;
    }
    strncpy(name, (char *)json_vars->json_token, len);

    if (ptype == STRUCT_WINDOW)
    {
        ((uC_window_t *)structure)->name = fnv_hash((uint8_t *)name);
        ((uC_window_t *)structure)->display_name = name;
        return;
    }

    if (ptype == STRUCT_MENU_ITEM)
    {
        ((menu_item_t *)structure)->name = name;
        return;
    }

    // ptype = STRUCT_PULLDOWN
    ((pulldown_t *)structure)->name  = name;
}

// -----------------------------------------------------------------------

static void val_m_item_flag(menu_item_t *item)
{
    if (json_vars->key_value == uC_MENU_DISABLED)
    {
        item->flags = json_vars->key_value;
        return;
    }

    json_error("Invalid menu-item flag type");
}

// -----------------------------------------------------------------------

static void val_pd_flag(pulldown_t *pd)
{
    if (json_vars->key_value == uC_MENU_DISABLED)
    {
        pd->flags = json_vars->key_value;
    }

    json_error("Invalid pulldown flag type");
}

// -----------------------------------------------------------------------

static void val_win_flag(uC_window_t *win)
{
    win->flags |= json_vars->key_value;

    if (json_vars->key_value == uC_WIN_BOXED)
    {
        if ((win->width & 0x8000) != 0)
        {
            win->width &= ~0x8000;
            win->width -= 2;
        }
        if ((win->height & 0x8000) != 0)
        {
            win->height &= ~0x8000;
            win->height -= 2;
        }
    }
}

// -----------------------------------------------------------------------

static void value_flag(void)
{
    json_state_t *gp;
    void *structure;
    json_type_t gtype;

    gp        = json_state->parent;
    gp        = gp->parent;
    structure = gp->structure;
    gtype     = gp->struct_type;

    if (gtype == STRUCT_MENU_ITEM)
    {
        val_m_item_flag(structure);
        return;
    }

    (gtype == STRUCT_PULLDOWN)
        ? val_pd_flag(structure)
        : val_win_flag(structure);   // gtype == STRUCT_WINDOW:
}

// -----------------------------------------------------------------------

static void value_border_type(void)
{
    json_state_t *parent = json_state->parent;
    uC_window_t  *win    = parent->structure;

    if ((json_vars->key_value == uC_BDR_SINGLE) ||
        (json_vars->key_value == uC_BDR_DOUBLE) ||
        (json_vars->key_value == uC_BDR_CURVED))
    {
        win->border_type = json_vars->key_value;
        return;
    }

    json_error("Invalid border type");
}

// -----------------------------------------------------------------------

static void value_blank(void)
{
    json_state_t *parent = json_state->parent;
    uC_window_t  *win    = parent->structure;

    win->blank = json_vars->key_value;
}

// -----------------------------------------------------------------------
// set tab selection order for a window

static void value_order(void)
{
    json_state_t *parent = json_state->parent;
    uC_window_t  *win    = parent->structure;

    win->tab_order = json_vars->key_value;
}

// -----------------------------------------------------------------------

static void value_vector(void)
{
    json_state_t *parent = json_state->parent;
    menu_item_t  *item   = parent->structure;

    if (json_vars->fp_finder != NULL)
    {
        if (json_vars->quoted == false)
        {
            json_error("Vector name must be quoted");
        }

        item->fp = (json_vars->fp_finder)(json_vars->json_hash);
    }
}

// -----------------------------------------------------------------------
// how == tbd

static void value_shortcut(void)
{
    json_state_t *parent = json_state->parent;
    menu_item_t  *item   = parent->structure;

    if (json_vars->quoted == false)
    {
        json_error("Shortcut value must be quoted");
    }

    if (json_vars->json_token[0] == '\0')
    {
        json_error("Shortcut value must not be empty");
    }

    item->shortcut = json_vars->json_token[0];
}

// -----------------------------------------------------------------------

static const uC_switch_t value_types[] =
{
    { KEY_FG,          value_attrib      },
    { KEY_BG,          value_attrib      },
    { KEY_GRAY_FG,     value_attrib      },
    { KEY_GRAY_BG,     value_attrib      },
    { KEY_RED,         value_rgb         },
    { KEY_GREEN,       value_rgb         },
    { KEY_BLUE,        value_rgb         },
    { KEY_XCO,         value_window_dim  },
    { KEY_YCO,         value_window_dim  },
    { KEY_WIDTH,       value_window_dim  },
    { KEY_HEIGHT,      value_window_dim  },
    { KEY_NAME,        value_name        },
    { KEY_FLAGS,       value_flag        },
    { KEY_BORDER_TYPE, value_border_type },
    { KEY_VECTOR,      value_vector      },
    { KEY_SHORTCUT,    value_shortcut    },
    { KEY_FLAG,        value_flag        },
    { KEY_BLANK,       value_blank       },
    { KEY_ORDER,       value_order       }
};

#define NUM_KEYS (sizeof(value_types) / sizeof(value_types[0]))

// -----------------------------------------------------------------------

static int32_t constant_hash[] =
{
    // uC_MENU_DISABLED  uC_BDR_SINGLE  uC_BDR_DOUBLE  uC_BDR_CURVED,
    0xd4d2d628,         0x19a4bb97,    0x59a80744,    0x08832084,
    // uC_WIN_LOCKED  uC_WIN_FILLED  uC_WIN_BOXED  uC_WIN_FAR,
    0xc58d5fe7,      0xe32fa129,      0xe03a18cf,    0x61b63280,
    // uC_WIN_FOCUS  uC_WIN_NAMED
    0xbbcce08f,     0xc1cbb244,

    // uC_COLOR_BLACK  uC_COLOR_RED    uC_COLOR_GREEN  uC_COLOR_BROWN
    0x353c0341,       0x1b8459ad,    0xd6a8fd81,      0xc3645914,
    // uC_COLOR_BLUE   uC_COLOR_MAGENTA  uC_COLOR_CYAN   uC_COLOR_WHITE
    0x043c5f94,       0x84d347dd,        0x88dc1e7d,     0x3b87c23f,
    // uC_COLOR_GRAY   uC_COLOR_LT_RED  uC_COLOR_LT_GREEN  uC_COLOR_YELLOW
    0x64e0ae05,       0xfbab764c,       0x05a9f85c,        0xe0283848,
    // uC_COLOR_LT_BLUE  uC_COLOR_LT_MAGENTA  uC_COLOR_LT_CYAN  uC_COLOR_LT_WHITE
    0x4d07c677,         0xdf77ab90,           0xf9424e06,       0x47891f9a
};

#define NUM_CONSTANTS (sizeof(constant_hash) / sizeof(constant_hash[0]))

static int32_t constant_val[] =
{
    uC_MENU_DISABLED, uC_BDR_SINGLE, uC_BDR_DOUBLE, uC_BDR_CURVED,
    uC_WIN_LOCKED,    uC_WIN_FILLED, uC_WIN_BOXED,  uC_WIN_FAR,
    uC_WIN_FOCUS,     uC_WIN_NAMED,

    // color values

    uC_COLOR_BLACK,    uC_COLOR_RED,
    uC_COLOR_GREEN,    uC_COLOR_BROWN,
    uC_COLOR_BLUE,     uC_COLOR_MAGENTA,
    uC_COLOR_CYAN,     uC_COLOR_WHITE,
    uC_COLOR_GRAY,     uC_COLOR_LT_RED,
    uC_COLOR_LT_GREEN, uC_COLOR_YELLOW,
    uC_COLOR_LT_BLUE,  uC_COLOR_LT_MAGENTA,
    uC_COLOR_LT_CYAN,  uC_COLOR_LT_WHITE
};

// -----------------------------------------------------------------------
// allows hex or deciamal but hex must be stated with lower case chars

static void parse_number(void)
{
    uint8_t c;
    uint8_t radix = 10;
    uint16_t i = 0;
    uint32_t result = 0;

    json_vars->percent = false;

    if ((json_vars->json_token[0] == '0') &&
        (json_vars->json_token[1] == 'x'))
    {
        radix = 16;
        i = 2;
    }

    while ((c = json_vars->json_token[i++]) != '\0')
    {
        if (c == '%')
        {
            json_vars->percent = true;
            break;
        }
        c -= '0';
        if (c > 9)
        {
            if (c > 17)
            {
                return; // key_value still == NAN
            }
            c -= 7;
        }
        result *= radix;
        result += c;
    }
    json_vars->key_value = result;
}

// -----------------------------------------------------------------------

static void is_constant(void)
{
    uint16_t i;

    for (i = 0; i < NUM_CONSTANTS; i++)
    {
        if (json_vars->json_hash == constant_hash[i])
        {
            json_vars->key_value = constant_val[i];
            break;
        }
    }
}

// -----------------------------------------------------------------------

static bool chk_quotes(uint16_t len)
{
    bool rv = false;

    if ((json_vars->json_token[0]       == '"') &&
        (json_vars->json_token[len - 1] == '"'))
    {
        rv = true;
        strip_quotes(len);
    }
    return rv;
}

// -----------------------------------------------------------------------

void json_state_value(void)
{
    bool has_comma = 0;
    size_t len;

    json_vars->key_value = UCURSES_NAN; // assume NAN

    len = strlen((char *)json_vars->json_token);

    if (json_vars->json_token[len - 1] == ',')
    {
        json_vars->json_token[--len] = '\0';
        has_comma = true;
    }

    json_vars->quoted = chk_quotes(len);

    is_constant();

    if (json_vars->key_value == UCURSES_NAN)
    {
        parse_number();
    }

    uC_switch(value_types, NUM_KEYS, json_state->struct_type);

    if (json_vars->json_stack.count != 0)
    {
        json_pop();

        json_state->state = (has_comma != 0)
            ? JSON_STATE_KEY
            : JSON_STATE_R_BRACE;
    }
    else // dont think this is possible but.... ya. this is C so it is
    {
        json_error("Whiskey Tango Foxtrot!");
    }
}

// =======================================================================

#endif
