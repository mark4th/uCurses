// json_value.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/uCurses.h"

#define FAR 0x264116fc  // not a constant value, needs to be calculated

// -----------------------------------------------------------------------

extern list_t j_stack;
extern j_state_t *j_state;
extern char json_token[TOKEN_LEN]; // space delimited token extracted from data
extern fp_finder_t fp_finder;
extern j_state_t *j_state;
extern uint32_t json_hash;

extern uint16_t console_width;
extern uint16_t console_height;

int32_t key_value;
uint16_t quoted;

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

    if((key_value <= 15) && (key_value > 0))
    {
        pstruct[FG]    = key_value;
        pstruct[ATTR] &= ~(BG_RGB | BG_GRAY);
        return;
    }
    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_bg(void)
{
    j_state_t *parent = j_state->parent;
    uint8_t *pstruct  = parent->structure;

    if((key_value <= 15) && (key_value > 0))
    {
        pstruct[FG] = key_value;
        pstruct[ATTR] &= ~(BG_RGB | BG_GRAY);
        return;
    }
    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_gray_fg(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if((key_value <= 23) && (key_value > 0))
    {
       pstruct[FG]    = key_value;
       pstruct[ATTR] |= FG_GRAY;
       pstruct[ATTR] &= ~FG_RGB;
       return;
    }
    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_gray_bg(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if((key_value <= 23) && (key_value > 0))
    {
        pstruct[BG]    = key_value;
        pstruct[ATTR] |= BG_GRAY;
        pstruct[ATTR] &= ~BG_RGB;
        return;
    }
    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_red(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if((key_value <= 255)  && (key_value > 0))
    {
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
        return;
    }
    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_green(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if((key_value <= 255) && (key_value > 0))
    {
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
        return;
    }
    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_blue(void)
{
    j_state_t *parent = j_state->parent;
    char *pstruct     = parent->structure;

    if((key_value <= 255) && (key_value > 0))
    {
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
        return;
    }
    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_xco(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if(key_value == FAR)
    {
        if(win->width == NAN)
        {
            json_error("Must know width to position FAR");
        }
        win->xco = console_width - win->width;
        if((win->flags & WIN_BOXED) != 0)
        {
            win->xco--;
        }
        return;
    }

    if((win->width + key_value) < console_width)
    {
        win->xco = key_value;
        return;
    }
    json_error("Window too far right to fit on console");
}

// -----------------------------------------------------------------------

static void value_yco(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if(key_value == FAR)
    {
        if(win->height == NAN)
        {
            json_error("Must know height to position FAR");
        }
        win->yco = console_height - win->height;
        if((win->flags & WIN_BOXED) != 0)
        {
            win->yco--;
        }
        return;
    }
    if((win->height + key_value) < console_height)
    {
        win->yco = key_value;
        return;
    }
    json_error("Window too far down to fit on console");
}

// -----------------------------------------------------------------------

static void value_width(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if(key_value < console_width)
    {
        win->width = key_value;
        return;
    }
    json_error("Window too wide to fit in console");
}

// -----------------------------------------------------------------------

static void value_height(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win     = parent->structure;

    if(key_value < console_height)
    {
        win->height = key_value;
        return;
    }
    json_error("Window too wide to fit in console");
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

    if(quoted == 0)  // was token quoted?
    {
        json_error("String key values must be quoted");
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
    j_state_t *parent;
    void *structure;
    uint32_t ptype;

    parent    = j_state->parent;
    parent    = j_state->parent;
    structure = parent->structure;
    ptype     = parent->struct_type;

    switch(ptype)
    {
        case STRUCT_MENU_ITEM:
            if(key_value == MENU_DISABLED)
            {
                ((menu_item_t *)structure)->flags = key_value;
                return;
            }
            json_error("Invalid flag type");
            break;
        case STRUCT_PULLDOWN:
            if(key_value == MENU_DISABLED)
            {
                ((pulldown_t *)structure)->flags = key_value;
            }
            json_error("Invalid flag type");
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
    screen_t *scr;

    if((key_value == BDR_SINGLE) ||
       (key_value == BDR_DOUBLE) ||
       (key_value == BDR_CURVED))
    {
        win->bdr_type = key_value;
        scr = win->screen;
        return;
    }
    json_error("Invalid border type");
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

    if(quoted == 0)
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
    { KEY_SHORTCUT,     value_shortcut    },
    { KEY_FLAG,         value_flag        }
};

#define NUM_KEYS (sizeof(value_types) / sizeof(value_types[0]))

// -----------------------------------------------------------------------

static uint32_t constant_hash[] =
{
    0x0ed8a8cf, 0xfa264646, 0x4e4f416d, 0x8cb49b59,
    0x901cbb7a, 0xd6b11d20, 0x6f7f7df8, 0x264116fc,

    // BLACK RED GREEN BROWN BLUE MAGENTA
    // CYAN WHITE GRAY PINK LT_GREEN YELLOW
    // LT_BLUE LT_MAGENTA CYAN LT_WHITE
    0xdc51d022, 0x5a235332, 0xe3671392, 0x4ff50adb,
    0xd1e100a9, 0x7dc1a602, 0x7cde54cc, 0xc2f8ecb8,
    0xbabf7ce4, 0xf62236fd, 0x064123b9, 0x4d265959,
    0x2805c15c, 0x186aeb45, 0x7cde54cc, 0x060a9a87
};

#define NUM_CONSTANTS (sizeof(constant_hash) / sizeof(constant_hash[0]))

static uint32_t constant_val[] =
{
    MENU_DISABLED, BDR_SINGLE, BDR_DOUBLE, BDR_CURVED,
    WIN_LOCKED,    WIN_FILLED, WIN_BOXED,  FAR,
    // color values
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

// -----------------------------------------------------------------------
// allowing hex or deciamal. but hex must be stated with lower case chars


// todo add percentages

static void parse_number(void)
{
    uint8_t c;
    uint8_t radix   = 10;
    uint16_t i      = 0;
    uint32_t result = 0;

    if((json_token[0] == '0') && (json_token[1] == 'x'))
    {
        radix = 16;
        i = 2;
    }

    while((c = json_token[i++]) != '\0')
    {
        c -= '0';
        if(c > 9)
        {
            if(c > 17)
            {
                return;  // key_value still == NAN
            }
            c -= 7;
        }
        result *= radix;
        result += c;
    }
    key_value = result;
}

// -----------------------------------------------------------------------

void json_state_value(void)
{
    uint32_t i;
    uint16_t has_comma = 0;
    uint16_t len;

    key_value = NAN;     // assume NAN
    quoted    = 0;      // true if key value is a string

    len = strlen(json_token);

    if(json_token[len -1] == ',')
    {
        json_token[len -1] = '\0';
        has_comma = 1;
        len--;
    }

    if((json_token[0]       == '"') ||
       (json_token[len - 1] == '"'))
    {
        quoted = 1;
        strip_quotes(len -2);
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

    if(key_value == NAN)
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
