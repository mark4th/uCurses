// json_value.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "uCurses.h"
#include "uC_json.h"
#include "uC_window.h"
#include "uC_utils.h"
#include "uC_switch.h"

// -----------------------------------------------------------------------

extern json_vars_t *json_vars;
extern json_state_t *json_state;

// -----------------------------------------------------------------------
// with a max menu item/bar name of 32 chars (way too long) this gives
// you a max of 64 items at full length (dont do it!)

// windows now have names too not just menus

char name_string_buff[2048];
int16_t nsi;

// -----------------------------------------------------------------------

static void value_fgbg(void)
{
    json_type_t ktype    = json_state->struct_type;
    json_state_t *parent = json_state->parent;
    int8_t *pstruct      = parent->structure;

    // assume setting bg

    int8_t mask = (int8_t) ~(BG_RGB | BG_GRAY);
    int8_t i = BG;

    if ((json_vars->key_value & ~255) == 0)
    {
        if (ktype == KEY_FG)
        {
            i = FG;
            mask = ~(FG_RGB | FG_GRAY);
        }

        pstruct[i] = json_vars->key_value;
        pstruct[ATTR] &= mask;

        return;
    }

    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_gray_fgbg(void)
{
    json_type_t ktype    = json_state->struct_type;
    json_state_t *parent = json_state->parent;
    int8_t *pstruct      = parent->structure;

    int8_t i   = BG;
    uint8_t m1 = BG_GRAY;
    uint8_t m2 = ~BG_RGB;

    if ((json_vars->key_value <= 23) && (json_vars->key_value >= 0))
    {
        if (ktype == KEY_GRAY_FG)
        {
            i = FG;
            m1 = FG_GRAY;
            m2 = ~FG_RGB;
        }

        pstruct[i]    = json_vars->key_value;
        pstruct[ATTR] |= m1;
        pstruct[ATTR] &= m2;

        return;
    }

    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_rgb_fg(char *gstruct)
{
    gstruct[ATTR] |= FG_RGB;
    gstruct[ATTR] &= ~FG_GRAY;

    if (json_state->struct_type == KEY_RED)
    {
        gstruct[FG_R] = json_vars->key_value;
    }
    else if (json_state->struct_type == KEY_GREEN)
    {
        gstruct[FG_G] = json_vars->key_value;
    }
    else
    {
        gstruct[FG_B] = json_vars->key_value;
    }
}

// -----------------------------------------------------------------------

static void value_rgb_bg(char *gstruct)
{
    gstruct[ATTR] |= BG_RGB;
    gstruct[ATTR] &= ~BG_GRAY;

    if (json_state->struct_type == KEY_RED)
    {
        gstruct[BG_R] = json_vars->key_value;
    }
    else if (json_state->struct_type == KEY_GREEN)
    {
        gstruct[BG_G] = json_vars->key_value;
    }
    else
    {
        gstruct[BG_B] = json_vars->key_value;
    }
    return;
}

// -----------------------------------------------------------------------

static void value_rgb(void)
{
    json_state_t *parent, *gp;
    int16_t ptype;
    char *gstruct;

    parent  = json_state->parent;  // rgb psudo structure
    gp      = parent->parent;      // attribs structure
    gstruct = gp->structure;       // really a char* of 8 bytes
    ptype   = parent->struct_type;

    if ((json_vars->key_value & ~255) == 0)
    {
        (ptype == STRUCT_RGB_FG)   // or its STRUCT_RGB_BG
            ? value_rgb_fg(gstruct)
            : value_rgb_bg(gstruct);
        return;
    }

    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_xy(void)
{
    json_state_t *parent = json_state->parent;
    window_t *win        = parent->structure;

    int16_t mult;

    if (json_vars->percent != false)
    {
        mult = (json_state->struct_type == KEY_XCO)
            ? json_vars->console_width
            : json_vars->console_height;
        json_vars->key_value *= mult;
        json_vars->key_value /= 100;
    }

    if (json_state->struct_type == KEY_XCO)
    {
        win->xco = json_vars->key_value;
    }
    else
    {
        win->yco = json_vars->key_value;
    }
}

// -----------------------------------------------------------------------

#define FILL 0xfa634be2

static void value_wh(void)
{
    json_state_t *parent = json_state->parent;

    window_t *win = parent->structure;

    int16_t mult;

    if (json_vars->percent != false)
    {
        mult = (json_state->struct_type == KEY_WIDTH)
            ? json_vars->console_width
            : json_vars->console_height;
        json_vars->key_value *= mult;
        json_vars->key_value /= 100;
        // mark for correction if window is given a border
        json_vars->key_value |= 0x8000;
    }

    if (json_state->struct_type == KEY_WIDTH)
    {
        win->width = json_vars->key_value;
    }
    else
    {
        win->height = json_vars->key_value;
    }
}

// -----------------------------------------------------------------------

static void value_name(void)
{
    uint16_t i;
    char *name;

    json_state_t *parent = json_state->parent;
    void *structure      = parent->structure;
    json_type_t ptype    = parent->struct_type;

    size_t len = strlen(json_vars->json_token);

    if (json_vars->quoted == false)  // was token quoted?
    {
        json_error("String key values must be quoted");
    }

    // copy name token to the name buff minus the quotes
    name = &name_string_buff[nsi];

    for (i = 0; i < len; i++)
    {
        name_string_buff[nsi++] = json_vars->json_token[i];
    }
    name_string_buff[nsi++] = '\0';

    if (ptype == STRUCT_WINDOW)
    {
        ((window_t *)structure)->name = fnv_hash(name);
    }
    else if (ptype == STRUCT_MENU_ITEM)
    {
        ((menu_item_t *)structure)->name = name;
    }
    else  // ptype = STRUCT_PULLDOWN
    {
        ((pulldown_t *)structure)->name  = name;
    }
}

// -----------------------------------------------------------------------

static void val_m_item_flag(menu_item_t *item)
{
    if (json_vars->key_value == MENU_DISABLED)
    {
        item->flags = json_vars->key_value;
        return;
    }

    json_error("Invalid flag type");
}

// -----------------------------------------------------------------------

static void val_pd_flag(pulldown_t *pd)
{
    if (json_vars->key_value == MENU_DISABLED)
    {
        pd->flags = json_vars->key_value;
    }

    json_error("Invalid flag type");
}

// -----------------------------------------------------------------------

static void val_win_flag(window_t *win)
{
    win->flags |= json_vars->key_value;

    if (json_vars->key_value == WIN_BOXED)
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
    }
    else if (gtype == STRUCT_PULLDOWN)
    {
        val_pd_flag(structure);
    }
    else // gtype == STRUCT_WINDOW:
    {
        val_win_flag(structure);
    }
}

// -----------------------------------------------------------------------

static void value_border_type(void)
{
    json_state_t *parent = json_state->parent;
    window_t *win = parent->structure;

    if ((json_vars->key_value == BDR_SINGLE) ||
        (json_vars->key_value == BDR_DOUBLE) ||
        (json_vars->key_value == BDR_CURVED))
    {
        win->bdr_type = json_vars->key_value;
        return;
    }

    json_error("Invalid border type");
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
    ;
}

// -----------------------------------------------------------------------

static const switch_t value_types[] =
{
    { KEY_FG,          value_fgbg        },
    { KEY_BG,          value_fgbg        },
    { KEY_GRAY_FG,     value_gray_fgbg   },
    { KEY_GRAY_BG,     value_gray_fgbg   },
    { KEY_RED,         value_rgb         },
    { KEY_GREEN,       value_rgb         },
    { KEY_BLUE,        value_rgb         },
    { KEY_XCO,         value_xy          },
    { KEY_YCO,         value_xy          },
    { KEY_WIDTH,       value_wh          },
    { KEY_HEIGHT,      value_wh          },
    { KEY_NAME,        value_name        },
    { KEY_FLAGS,       value_flag        },
    { KEY_BORDER_TYPE, value_border_type },
    { KEY_VECTOR,      value_vector      },
    { KEY_SHORTCUT,    value_shortcut    },
    { KEY_FLAG,        value_flag        }
};

#define NUM_KEYS (sizeof(value_types) / sizeof(value_types[0]))

// -----------------------------------------------------------------------

static int32_t constant_hash[] =
{
    // MENU_DISABLED  BDR_SINGLE   BDR_DOUBLE   BDR_CURVED,
    0x0ed8a8cf,       0xfa264646,  0x4e4f416d,  0x8cb49b59,
    // WIN_LOCKED     WIN_FILLED   WIN_BOXED    WIN_FAR,
    0x901cbb7a,       0xd6b11d20,  0x6f7f7df8,  0x264116fc,


    // BLACK    RED         GREEN       BROWN
    0xdc51d022, 0x5a235332, 0xe3671392, 0x4ff50adb,
    // BLUE     MAGENTA     CYAN        WHITE
    0xd1e100a9, 0x7dc1a602, 0x7cde54cc, 0xc2f8ecb8,
    // GRAY     PINK        LT_GREEN    YELLOW
    0xbabf7ce4, 0xf62236fd, 0x064123b9, 0x4d265959,
    // LT_BLUE  LT_MAGENTA  CYAN        LT_WHITE
    0x2805c15c, 0x186aeb45, 0x7cde54cc, 0x060a9a87
};

#define NUM_CONSTANTS (sizeof(constant_hash) / sizeof(constant_hash[0]))

static int32_t constant_val[] =
{
    MENU_DISABLED, BDR_SINGLE, BDR_DOUBLE, BDR_CURVED,
    WIN_LOCKED,    WIN_FILLED, WIN_BOXED,  WIN_FAR,

    // color values
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

// -----------------------------------------------------------------------
// allowing hex or deciamal. but hex must be stated with lower case chars

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

    len = strlen(json_vars->json_token);

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

    re_switch(value_types, NUM_KEYS, json_state->struct_type);

    if (json_vars->json_stack.count != 0)
    {
        json_pop();

        json_state->state = (has_comma != 0)
            ? STATE_KEY
            : STATE_R_BRACE;
    }
    else // dont think this is possible but.... ya. this is C so it is
    {
        json_error("Whiskey Tango Foxtrot!");
    }
}

// =======================================================================
