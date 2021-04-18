// json_value.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

extern list_t j_stack;
extern j_state_t *j_state;
extern char json_token[TOKEN_LEN]; // space delimited token extracted from data
extern fp_finder_t fp_finder;
extern j_state_t *j_state;
extern uint32_t json_hash;

static uint16_t console_width;
static uint16_t console_height;

int32_t key_value;

// -----------------------------------------------------------------------
// with a max menu item/bar name of 32 chars (way too long) this gives
// you a max of 64 items at full length (dont do it!)

char name_string_buff[2048];
uint16_t nsi;

// -----------------------------------------------------------------------

static void value_fg(void)
{
    j_state_t *parent  = j_state->parent;
    uint8_t *pstruct = parent->structure;

    if(key_value > 15)
    {
        json_error("Value out of range");
    }

    pstruct[FG] = key_value;
    pstruct[ATTR] &= ~(BG_RGB | BG_GRAY);
}

// -----------------------------------------------------------------------

static void value_bg(void)
{
    j_state_t *parent = j_state->parent;
    uint8_t *pstruct  = parent->structure;

    if(key_value > 15)
    {
        json_error("Value out of range");
    }

    pstruct[FG] = key_value;
    pstruct[ATTR] &= ~(BG_RGB | BG_GRAY);
}

// -----------------------------------------------------------------------

static void value_gray_fg(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if(key_value > 23)
    {
        json_error("Value out of range");
    }

    pstruct[FG]    = key_value;
    pstruct[ATTR] |= FG_GRAY;
    pstruct[ATTR] &= ~FG_RGB;
}

// -----------------------------------------------------------------------

static void value_gray_bg(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if(key_value > 23)
    {
        json_error("Value out of range");
    }

    pstruct[BG]    = key_value;
    pstruct[ATTR] |= BG_GRAY;
    pstruct[ATTR] &= ~BG_RGB;
}

// -----------------------------------------------------------------------

static void value_red(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if(key_value > 255)
    {
        json_error("Value out of range");
    }

    if(j_state->struct_type == STRUCT_RGB_FG)
    {
        pstruct[FG_R]  = key_value;
        pstruct[ATTR] |= FG_RGB;
        pstruct[ATTR] &= ~FG_GRAY;
    }
    else
    {
        pstruct[BG_R]  = key_value;
        pstruct[ATTR] |= BG_RGB;
        pstruct[ATTR] &= ~BG_GRAY;
    }
}

// -----------------------------------------------------------------------

static void value_green(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if(key_value > 255)
    {
        json_error("Value out of range");
    }
    if(j_state->struct_type == STRUCT_RGB_FG)
    {
        pstruct[FG_G] = key_value;
        pstruct[ATTR] |= FG_RGB;
        pstruct[ATTR] &= ~FG_GRAY;
    }
    else
    {
        pstruct[BG_G] = key_value;
        pstruct[ATTR] |= BG_RGB;
        pstruct[ATTR] &= ~BG_GRAY;
    }
}

// -----------------------------------------------------------------------

static void value_blue(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if(key_value > 255)
    {
        json_error("Value out of range");
    }

    if(j_state->struct_type == STRUCT_RGB_FG)
    {
        pstruct[FG_B]  = key_value;
        pstruct[ATTR] |= FG_RGB;
        pstruct[ATTR] &= ~FG_GRAY;
    }
    else
    {
        pstruct[BG_B] = key_value;
        pstruct[ATTR] |= BG_RGB;
        pstruct[ATTR] &= ~BG_GRAY;
    }
}

// -----------------------------------------------------------------------

static void value_xco(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if((win->width + key_value) > console_width)
    {
        json_error("Window too far right to fit on console");
    }
    win->xco = key_value;
}

// -----------------------------------------------------------------------

static void value_yco(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if((win->height + key_value) > console_height)
    {
        json_error("Window too far down to fit on console");
    }
    win->yco = key_value;
}

// -----------------------------------------------------------------------

static void value_width(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if(key_value > console_width)
    {
        json_error("Window too wide to fit in console");
    }
    win->width = key_value;
}

// -----------------------------------------------------------------------

static void value_height(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if(key_value > console_height)
    {
        json_error("Window too wide to fit in console");
    }
    win->height = key_value;
}

// -----------------------------------------------------------------------

static void value_name(void)
{
    uint16_t i;
    char *name;

    j_state_t *parent = j_state->parent;
    void *structure   = parent->structure;
    uint32_t ptype    = parent->struct_type;

    size_t len = strlen(json_token);

    if((json_token[0]       != '"') &&
       (json_token[len - 1] != '"'))
    {
        json_error("String key values must be quoted");
    }

    if(len > 34)
    {
        json_error("Name string too long (max 32)");
    }

    // copy name token to the name buff minus the quotes
    name = &name_string_buff[nsi];
    for(i = 0; i < len - 2; i++)
    {
        name_string_buff[nsi++] = json_token[i + 1];
    }
    name_string_buff[nsi++] = '\0';

    switch(ptype)
    {
        case STRUCT_MENU_ITEM:
            ((menu_item_t *)structure)->name = name;
            break;
        case STRUCT_PULLDOWN:
            ((pulldown_t *)structure)->name = name;
            break;
    }
}

// -----------------------------------------------------------------------

static void value_flag(void)
{
    j_state_t *parent = j_state->parent;
    void *structure   = parent->structure;
    uint32_t ptype    = parent->struct_type;

    if(key_value == MENU_DISABLED)
    {
        if((ptype != STRUCT_MENU_ITEM) &&
           (ptype != STRUCT_PULLDOWN))
        {
            json_error("Invalid flag type");
        }
    }
    else if((ptype == STRUCT_MENU_ITEM) ||
            (ptype == STRUCT_PULLDOWN))
    {
        json_error("Invalid flag type");
    }

    switch(ptype)
    {
        case STRUCT_MENU_ITEM:
            ((menu_item_t *)structure)->flags = key_value;
            break;
        case STRUCT_PULLDOWN:
            ((pulldown_t *)structure)->flags = key_value;
            break;
        case STRUCT_WINDOW:
            ((window_t *)structure)->flags |= key_value;
            break;
    }
}

// -----------------------------------------------------------------------

static void value_border_type(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if((key_value != WIN_LOCKED) && (key_value != WIN_FILLED) &&
        (key_value != WIN_BOXED))
    {
        json_error("Invalid border type");
    }
    win->bdr_type = key_value;
}

// -----------------------------------------------------------------------

static void value_vector(void)
{
    j_state_t *parent = j_state->parent;
    menu_item_t *item = parent->structure;

    if(fp_finder == NULL)
    {
        return;
    }
    uint16_t len = strlen(json_token);

    if((json_token[0]       != '"') &&
       (json_token[len - 1] != '"'))
    {
        json_error("Function name must be quoted");
    }

    strip_quotes(len -2);

    item->fp = (fp_finder)(json_hash);

    if(item->fp == NULL)
    {
        json_error("Unknown function");
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
    { KEY_FG,           value_fg          },
    { KEY_BG,           value_bg          },
    { KEY_GRAY_FG,      value_gray_fg     },
    { KEY_GRAY_BG,      value_gray_bg     },
    { KEY_RED,          value_red         },
    { KEY_GREEN,        value_green       },
    { KEY_BLUE,         value_blue        },
    { KEY_XCO,          value_xco         },
    { KEY_YCO,          value_yco         },
    { KEY_WIDTH,        value_width       },
    { KEY_HEIGHT,       value_height      },
    { KEY_NAME,         value_name        },
    { KEY_FLAGS,        value_flag        },
    { KEY_BORDER_TYPE,  value_border_type },
    { KEY_VECTOR,       value_vector      },
    { KEY_SHORTCUT,     value_shortcut    }
};

#define NUM_KEYS (sizeof(value_types) / sizeof(value_types[0]))

// -----------------------------------------------------------------------

static uint32_t constant_hash[] =
{
    0x0ed8a8cf, 0xfa264646, 0x4e4f416d, 0x8cb49b59,
    0x901cbb7a, 0xd6b11d20, 0x6f7f7df8
};

#define NUM_CONSTANTS (sizeof(constant_hash) / sizeof(constant_hash[0]))

static uint32_t constant_val[] =
{
    MENU_DISABLED, BDR_SINGLE, BDR_DOUBLE, BDR_CURVED,
    WIN_LOCKED,    WIN_FILLED, WIN_BOXED
};

// -----------------------------------------------------------------------
// allowing hex or deciamal.

// todo - just being lazy, this is ezpz
static void parse_number(void)
{
    // parse token here... blah blah

    if(key_value == -1)
    {
        json_error("Not a valid numerical value");
    }
}

// -----------------------------------------------------------------------

void json_state_value(void)
{
    uint32_t i;
    uint16_t has_comma = 0;
    uint16_t len = strlen(json_token) - 1;

    key_value = -1;     // assume NAN

    token();            // might be a number or a known constant

    if(json_token[len] == ',')
    {
        json_token[len] = '\0';
        has_comma = 1;
    }

    json_hash = fnv_hash(json_token);

    for(i = 0; i < NUM_CONSTANTS; i++)
    {
        if(json_hash == constant_hash[i])
        {
            key_value = constant_val[i];
            break;
        }
    }

    if((json_token[0] != '"') && (key_value == -1))
    {
        parse_number();
    }

    // we got this far, one of these will work
    re_switch(value_types, NUM_KEYS, j_state->struct_type);

    if(j_stack.count != 0)
    {
        j_pop();
        j_state->state = (has_comma != 0)
            ? STATE_KEY
            : STATE_R_BRACE;
    }
    else  // dont think this is possible but.... ya. this is C so it is
    {
        json_error("Whiskey Tango Foxtrot!");
    }
}

// =======================================================================
