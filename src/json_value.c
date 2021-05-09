// json_value.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

extern list_t j_stack;
extern j_state_t *j_state;
extern char json_token[TOKEN_LEN];
extern fp_finder_t fp_finder;
extern j_state_t *j_state;
extern int32_t json_hash;

extern int16_t console_width;
extern int16_t console_height;

static int16_t percent;

int32_t key_value;
int16_t quoted;

// -----------------------------------------------------------------------
// with a max menu item/bar name of 32 chars (way too long) this gives
// you a max of 64 items at full length (dont do it!)

char name_string_buff[2048];
int16_t nsi;

// -----------------------------------------------------------------------

static void value_fgbg(void)
{
    int16_t ktype = j_state->struct_type;
    j_state_t *parent = j_state->parent;
    int8_t *pstruct = parent->structure;

    // assume setting bg

    int8_t mask = (int8_t) ~(BG_RGB | BG_GRAY);
    int8_t i = BG;

    if((key_value & ~255) == 0)
    {
        if(ktype == KEY_FG)
        {
            i = FG;
            mask = ~(FG_RGB | FG_GRAY);
        }

        pstruct[i] = key_value;
        pstruct[ATTR] &= mask;

        return;
    }

    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_gray_fgbg(void)
{
    int16_t ktype = j_state->struct_type;
    j_state_t *parent = j_state->parent;
    int8_t *pstruct = parent->structure;
    int8_t i = BG;
    uint8_t m1 = BG_GRAY;
    uint8_t m2 = ~BG_RGB;

    if((key_value <= 23) && (key_value >= 0))
    {
        if(ktype == KEY_GRAY_FG)
        {
            i = FG;
            m1 = FG_GRAY;
            m2 = ~FG_RGB;
        }

        pstruct[i] = key_value;
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

    if(j_state->struct_type == KEY_RED)
    {
        gstruct[FG_R] = key_value;
    }
    else if(j_state->struct_type == KEY_GREEN)
    {
        gstruct[FG_G] = key_value;
    }
    else
    {
        gstruct[FG_B] = key_value;
    }
}

// -----------------------------------------------------------------------

static void value_rgb_bg(char *gstruct)
{
    gstruct[ATTR] |= BG_RGB;
    gstruct[ATTR] &= ~BG_GRAY;

    if(j_state->struct_type == KEY_RED)
    {
        gstruct[BG_R] = key_value;
    }
    else if(j_state->struct_type == KEY_GREEN)
    {
        gstruct[BG_G] = key_value;
    }
    else
    {
        gstruct[BG_B] = key_value;
    }
    return;
}

// -----------------------------------------------------------------------

static void value_rgb(void)
{
    j_state_t *parent, *gp;
    int16_t ptype;
    char *gstruct;

    parent = j_state->parent; // rgb psudo structure
    gp = parent->parent;      // attribs structure
    gstruct = gp->structure;  // really a char* of 8 bytes
    ptype = parent->struct_type;

    if((key_value <= 255) && (key_value >= 0))
    {
        (ptype == STRUCT_RGB_FG) // or its STRUCT_RGB_BG
            ? value_rgb_fg(gstruct)
            : value_rgb_bg(gstruct);
        return;
    }

    json_error("Value out of range");
}

// -----------------------------------------------------------------------

static void value_xy(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win = parent->structure;

    if(j_state->struct_type == KEY_XCO)
    {
        if(percent != 0)
        {
            key_value *= console_width;
            key_value /= 100;
        }
        win->xco = key_value;
    }
    else
    {
        if(percent != 0)
        {
            key_value *= console_height;
            key_value /= 100;
        }
        win->yco = key_value;
    }
}

// -----------------------------------------------------------------------

static void value_wh(void)
{
    j_state_t *parent = j_state->parent;
    window_t *win = parent->structure;

    if(j_state->struct_type == KEY_WIDTH)
    {
        if(percent != 0)
        {
            key_value *= console_width;
            key_value /= 100;
        }
        win->width = key_value;
    }
    else // KEY_HEIGHT
    {
        if(percent != 0)
        {
            key_value *= console_height;
            key_value /= 100;
        }
        win->height = key_value;
    }
}

// -----------------------------------------------------------------------

static void value_name(void)
{
    uint16_t i;
    char *name;

    j_state_t *parent = j_state->parent;
    void *structure = parent->structure;
    int32_t ptype = parent->struct_type;

    size_t len = strlen(json_token);

    if(quoted == 0) // was token quoted?
    {
        json_error("String key values must be quoted");
    }

    // copy name token to the name buff minus the quotes
    name = &name_string_buff[nsi];
    for(i = 0; i < len; i++)
    {
        name_string_buff[nsi++] = json_token[i];
    }
    name_string_buff[nsi++] = '\0';

    if(ptype == STRUCT_MENU_ITEM)
    {
        ((menu_item_t *)structure)->name = name;
    }
    else // ptype == STRUCT_PULLDOWN:
    {
        ((pulldown_t *)structure)->name = name;
    }
}

// -----------------------------------------------------------------------

static void val_m_item_flag(menu_item_t *item)
{
    if(key_value == MENU_DISABLED)
    {
        item->flags = key_value;
        return;
    }

    json_error("Invalid flag type");
}

// -----------------------------------------------------------------------

static void val_pd_flag(pulldown_t *pd)
{
    if(key_value == MENU_DISABLED)
    {
        pd->flags = key_value;
    }

    json_error("Invalid flag type");
}

// -----------------------------------------------------------------------

static void val_win_flag(window_t *win) { win->flags |= key_value; }

// -----------------------------------------------------------------------

static void value_flag(void)
{
    j_state_t *gp;
    void *structure;
    int32_t gtype;

    gp = j_state->parent;
    gp = gp->parent;
    structure = gp->structure;
    gtype = gp->struct_type;

    if(gtype == STRUCT_MENU_ITEM)
    {
        val_m_item_flag(structure);
    }
    else if(gtype == STRUCT_PULLDOWN)
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
    j_state_t *parent = j_state->parent;
    window_t *win = parent->structure;

    if((key_value == BDR_SINGLE) || (key_value == BDR_DOUBLE) ||
       (key_value == BDR_CURVED))
    {
        win->bdr_type = key_value;
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

    if(quoted == 0)
    {
        json_error("Vector name must be quoted");
    }

    item->fp = (fp_finder)(json_hash);
}

// -----------------------------------------------------------------------
// how == tbd

static void value_shortcut(void) { ; }

// -----------------------------------------------------------------------

static const switch_t value_types[] = {
    // this comment is to resoluve a bug in clang-format
    // that would push everything in here out to column 41
    { KEY_FG, value_fgbg },
    { KEY_BG, value_fgbg },
    { KEY_GRAY_FG, value_gray_fgbg },
    { KEY_GRAY_BG, value_gray_fgbg },
    { KEY_RED, value_rgb },
    { KEY_GREEN, value_rgb },
    { KEY_BLUE, value_rgb },
    { KEY_XCO, value_xy },
    { KEY_YCO, value_xy },
    { KEY_WIDTH, value_wh },
    { KEY_HEIGHT, value_wh },
    { KEY_NAME, value_name },
    { KEY_FLAGS, value_flag },
    { KEY_BORDER_TYPE, value_border_type },
    { KEY_VECTOR, value_vector },
    { KEY_SHORTCUT, value_shortcut },
    { KEY_FLAG, value_flag }
};

#define NUM_KEYS (sizeof(value_types) / sizeof(value_types[0]))

// -----------------------------------------------------------------------

static int32_t constant_hash[] = {
    0x0ed8a8cf, 0xfa264646, 0x4e4f416d, 0x8cb49b59, 0x901cbb7a, 0xd6b11d20,
    0x6f7f7df8, 0x264116fc,

    // BLACK RED GREEN BROWN BLUE MAGENTA
    // CYAN WHITE GRAY PINK LT_GREEN YELLOW
    // LT_BLUE LT_MAGENTA CYAN LT_WHITE

    0xdc51d022, 0x5a235332, 0xe3671392, 0x4ff50adb, 0xd1e100a9, 0x7dc1a602,
    0x7cde54cc, 0xc2f8ecb8, 0xbabf7ce4, 0xf62236fd, 0x064123b9, 0x4d265959,
    0x2805c15c, 0x186aeb45, 0x7cde54cc, 0x060a9a87
};

#define NUM_CONSTANTS (sizeof(constant_hash) / sizeof(constant_hash[0]))

static int32_t constant_val[] = {
    // this comment is to resoluve a bug in clang-format

    MENU_DISABLED, BDR_SINGLE, BDR_DOUBLE, BDR_CURVED, WIN_LOCKED,
    WIN_FILLED, WIN_BOXED, FAR,

    // color values
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};

// -----------------------------------------------------------------------
// allowing hex or deciamal. but hex must be stated with lower case chars

// todo add percentages

static INLINE void parse_number(void)
{
    uint8_t c;
    uint8_t radix = 10;
    uint16_t i = 0;
    uint32_t result = 0;

    percent = 0;

    if((json_token[0] == '0') && (json_token[1] == 'x'))
    {
        radix = 16;
        i = 2;
    }

    while((c = json_token[i++]) != '\0')
    {
        if(c == '%')
        {
            percent = 1;
            break;
        }
        c -= '0';
        if(c > 9)
        {
            if(c > 17)
            {
                return; // key_value still == NAN
            }
            c -= 7;
        }
        result *= radix;
        result += c;
    }
    key_value = result;
}

// -----------------------------------------------------------------------

static INLINE void is_constant(void)
{
    uint16_t i;

    for(i = 0; i < NUM_CONSTANTS; i++)
    {
        if(json_hash == constant_hash[i])
        {
            key_value = constant_val[i];
            break;
        }
    }
}

// -----------------------------------------------------------------------

static INLINE uint16_t chk_quotes(uint16_t len)
{
    int16_t rv = 0;

    if((json_token[0] == '"') && (json_token[len - 1] == '"'))
    {
        rv = 1;
        strip_quotes(len);
    }
    return rv;
}

// -----------------------------------------------------------------------

void json_state_value(void)
{
    int16_t has_comma = 0;
    size_t len;

    key_value = NAN; // assume NAN

    len = strlen(json_token);

    if(json_token[len - 1] == ',')
    {
        json_token[--len] = '\0';
        has_comma = 1;
    }

    quoted = chk_quotes(len);

    is_constant();

    if(key_value == NAN)
    {
        parse_number();
    }

    re_switch(value_types, NUM_KEYS, j_state->struct_type);

    if(j_stack.count != 0)
    {
        j_pop();

        j_state->state = (has_comma != 0) ? STATE_KEY : STATE_R_BRACE;
    }
    else // dont think this is possible but.... ya. this is C so it is
    {
        json_error("Whiskey Tango Foxtrot!");
    }
}

// =======================================================================
