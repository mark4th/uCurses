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
    j_state_t *parent = j_state->parent;
    uint8_t *structure   = parent->structure;

    if(key_value > 15)
    {
        json_error("Value out of range");
    }

    structure[FG] = key_value;
    structure[ATTR] &= ~(BG_RGB | BG_GRAY);
}

// -----------------------------------------------------------------------

static void value_bg(void)
{
    j_state_t *parent = j_state->parent;
    uint8_t *structure   = parent->structure;

    if(key_value > 15)
    {
        json_error("Value out of range");
    }

    structure[FG] = key_value;
    structure[ATTR] &= ~(BG_RGB | BG_GRAY);
}

// -----------------------------------------------------------------------

static void value_gray_fg(void)
{
    j_state_t *parent = j_state->parent;
    char *structure   = parent->structure;

    if(key_value > 23)
    {
        json_error("Value out of range");
    }

    structure[FG] = key_value;
    structure[ATTR] |= FG_GRAY;
    structure[ATTR] &= ~FG_RGB;
}

// -----------------------------------------------------------------------

static void value_gray_bg(void)
{
    j_state_t *parent = j_state->parent;
    char *structure   = parent->structure;

    if(key_value > 23)
    {
        json_error("Value out of range");
    }

    structure[BG] = key_value;
    structure[ATTR] |= BG_GRAY;
    structure[ATTR] &= ~BG_RGB;
}

// -----------------------------------------------------------------------

static void value_red(void)
{
    j_state_t *parent = j_state->parent;
    char *structure   = parent->structure;

    if(key_value > 255)
    {
        json_error("Value out of range");
    }

    if(j_state->struct_type == STRUCT_RGB_FG)
    {
        structure[FG_R] = key_value;
        structure[ATTR] |= FG_RGB;
        structure[ATTR] &= ~FG_GRAY;
    }
    else
    {
        structure[BG_R] = key_value;
        structure[ATTR] |= BG_RGB;
        structure[ATTR] &= ~BG_GRAY;
    }
}

// -----------------------------------------------------------------------

static void value_green(void)
{
    j_state_t *parent = j_state->parent;
    char *structure   = parent->structure;

    if(key_value > 255)
    {
        json_error("Value out of range");
    }
    if(j_state->struct_type == STRUCT_RGB_FG)
    {
        structure[FG_G] = key_value;
        structure[ATTR] |= FG_RGB;
        structure[ATTR] &= ~FG_GRAY;
    }
    else
    {
        structure[BG_G] = key_value;
        structure[ATTR] |= BG_RGB;
        structure[ATTR] &= ~BG_GRAY;
    }
}

// -----------------------------------------------------------------------

static void value_blue(void)
{
    j_state_t *parent = j_state->parent;
    char *structure   = parent->structure;

    if(key_value > 255)
    {
        json_error("Value out of range");
    }

    if(j_state->struct_type == STRUCT_RGB_FG)
    {
        structure[FG_B] = key_value;
        structure[ATTR] |= FG_RGB;
        structure[ATTR] &= ~FG_GRAY;
    }
    else
    {
        structure[BG_B] = key_value;
        structure[ATTR] |= BG_RGB;
        structure[ATTR] &= ~BG_GRAY;
    }
}

// -----------------------------------------------------------------------

static void value_xco(void)
{
    j_state_t *parent  = j_state->parent;
    window_t *win = parent->structure;

    if((win->width + key_value) > console_width)
    {
        json_error("Window too far right to fit on console");
    }
    win->xco = key_value;
}

// -----------------------------------------------------------------------

static void value_yco(void)
{
    j_state_t *parent  = j_state->parent;
    window_t *win = parent->structure;

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
    window_t *win = parent->structure;

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
    window_t *win = parent->structure;

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
        if((ptype != STRUCT_MENU_ITEM) && (ptype != STRUCT_PULLDOWN))
        {
            json_error("Invalid flag type");
        }
    }
    else if((ptype == STRUCT_MENU_ITEM) || (ptype == STRUCT_PULLDOWN))
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
    window_t *win = parent->structure;

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
    int i;

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

    // strip qutes from token and recalculate hash
    for(i = 0; i < len - 2; i++)
    {
        json_token[i] = json_token[i + 1];
    }
    json_token[i] = '\0';
    json_hash = fnv_hash(json_token);

    item->fp = fp_finder(json_hash);

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
    { 0x6b77251c,  value_fg          },
    { 0xaa3b6788,  value_gray_fg     },
    { 0x6f772ba0,  value_bg          },
    { 0xa63b61c4,  value_gray_bg     },
    { 0x3a72d292,  value_red         },
    { 0xf73297b2,  value_green       },
    { 0x4f068569,  value_blue        },
    { 0x1c63995d,  value_xco         },
    { 0x3461800c,  value_yco         },
    { 0x182e64eb,  value_width       },
    { 0x4c47d5c0,  value_height      },
    { 0x2f8b3bf4,  value_name        },
    { 0x362bb2fc,  value_border_type },
    { 0x0ee694b4,  value_vector      },
    { 0x1c13e01f,  value_shortcut    },
    { 0xaeb95d5b,  value_flag        }
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
    re_switch(value_types, NUM_KEYS, json_hash);

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
