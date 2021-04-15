// uCurses json parsing for user interface layout
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "h/uCurses.h"
#include "h/list.h"

// -----------------------------------------------------------------------

char *json_data;        // pointer to json data to be parsed
size_t json_len;        // total size of json data
uint32_t json_index;    // parse index into data (current line)

char line_buff[MAX_LINE_LEN];
uint16_t line_no;
uint16_t line_index;    // line parse location
uint16_t line_left;     // number of chars left to parse in line

char json_token[TOKEN_LEN]; // space delimited token extracted from data
uint32_t json_hash;
uint16_t array;         // true if structure is an array
int32_t key_value;

static list_t j_stack;

static uint16_t console_width;
static uint16_t console_height;

// -----------------------------------------------------------------------

typedef enum
{
    STATE_L_BRACE,
    STATE_KEY,
    STATE_VALUE,
    STATE_R_BRACE,
    STATE_DONE,
    NUM_STATES
} json_state_t;

// -----------------------------------------------------------------------
// json syntax

const uint32_t json_syntax[] =
{
//    0x050c5d3c,             // #
    0x050c5d25,             // :
    0x050c5d64,             // {
    0x050c5d62,             // }
    0x050c5d44,             // [
    0x050c5d42              // ]
};

// pfffsets into above array of hash values
typedef enum
{
    // JSON_COMMENT,
    JSON_COLON,
    JSON_L_BRACE,
    JSON_R_BRACE,
    JSON_L_BRACKET,
    JSON_R_BRACKET
} json_syntax_t;

// -----------------------------------------------------------------------

typedef enum
{
    STRUCT_SCREEN = 1,
    STRUCT_WINDOWS,
    STRUCT_WINDOW,
    STRUCT_BACKDROP,
    STRUCT_MENU_BAR,
    STRUCT_PULLDOWNS,
    STRUCT_PULLDOWN,
    STRUCT_MENU_ITEMS,
    STRUCT_MENU_ITEM,
    STRUCT_ATTRIBS,         // normal attribs
    STRUCT_B_ATTRIBS,       // border attribs
    STRUCT_S_ATTRIBS,       // selected attribs
    STRUCT_D_ATTRIBS,       // disabled attribs
    STRUCT_RGB_FG,          // an optional array of 3 bytes
    STRUCT_RGB_BG,          // an optional array of 3 bytes
    STRUCT_FLAGS,
    KEY_FG,
    KEY_BG,
    KEY_GRAY_BG,
    KEY_RED,
    KEY_GREEN,
    KEY_BLUE,
    KEY_XCO,
    KEY_YCO,
    KEY_WIDTH,
    KEY_HEIGHT,
    KEY_NAME,
    KEY_FLAGS,
    KEY_BORDER_TYPE,
    KEY_VECTOR,
    KEY_SHORTCUT
} key_type_t;

// -----------------------------------------------------------------------

j_state_t *j_state;

// -----------------------------------------------------------------------

static void j_push(j_state_t *j)
{
    list_append_node(&j_stack, j_state);
    j_state = j;
}

// -----------------------------------------------------------------------

static void j_pop(void)
{
    free(j_state);

    j_state = list_pop(&j_stack);
}

// -----------------------------------------------------------------------

void json_error(char *s)
{
    // printf offending line and pointer to offending item?
    fprintf(stderr, "%d:%d %s\n", line_no, line_index, s);
    _exit(1);
}

// -----------------------------------------------------------------------

void *j_alloc(uint32_t size)
{
    void *v = calloc(1, size);

    if(v == NULL)
    {
        json_error("Out of Memory!");
    }
    return v;
}

// -----------------------------------------------------------------------
// json files and individual structures must start with a {

static void state_l_brace(void)
{
    if(array == 1)
    {
        if(json_hash != json_syntax[JSON_L_BRACKET])
        {
            json_error("Missing opening bracket");
        }
        // require closing bracket too
        array = 2;
    }
    else if(json_hash != json_syntax[JSON_L_BRACE])
    {
        json_error("Opening brace missing");
    }

    j_state->state++;
}

// -----------------------------------------------------------------------

static void new_state_struct(size_t struct_size, uint32_t struct_type,
    uint32_t state)
{
    j_state_t *j;

    void *structure = NULL;

    j = j_alloc(sizeof(*j));

    if(struct_size != 0)
    {
        structure = j_alloc(struct_size);
    }

    j->parent      = j_state;
    j->structure   = structure;
    j->struct_type = struct_type;
    j->state       = state;

    j_push(j);
}

// -----------------------------------------------------------------------
// must be the first struture specified

static void struct_screen(void)
{
    if(j_stack.count != 0)
    {
        json_error("There can be only one screen");
    }

    // on completion of parsing in this screen our parent structure
    // myst have a closing brace - after which we are done

    j_state->state = STATE_R_BRACE;
    new_state_struct(sizeof(screen_t), STRUCT_SCREEN, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_windows(void)
{
    if(j_state->struct_type != STRUCT_SCREEN)
    {
        json_error("Requires parent screen");
    }

    new_state_struct(0, STRUCT_WINDOWS, STATE_L_BRACE);
}

// -----------------------------------------------------------------------
// for there to be a window there must first be a screen

static void struct_window(void)
{
    if(j_state->struct_type != STRUCT_WINDOWS)
    {
        json_error("Requires parent windows structure");
    }

    new_state_struct(sizeof(window_t), STRUCT_WINDOW, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_backdrop(void)
{
    if(j_state->struct_type != STRUCT_SCREEN)
    {
        json_error("Requires parent screen");
    }

    new_state_struct(sizeof(window_t), STRUCT_BACKDROP, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_m_bar(void)
{
    if(j_state->struct_type != STRUCT_SCREEN)
    {
        json_error("Requires parent screen");
    }

    new_state_struct(sizeof(menu_bar_t), STRUCT_MENU_BAR, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_pulldowns(void)
{
    if(j_state->struct_type != STRUCT_MENU_BAR)
    {
        json_error("Requires parent menu-bar");
    }

    new_state_struct(0, STRUCT_PULLDOWNS, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_pulldown(void)
{
    if(j_state->struct_type != STRUCT_PULLDOWNS)
    {
        json_error("Requires parent pulldowns structure");
    }

    new_state_struct(sizeof(pulldown_t), STRUCT_PULLDOWN, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_m_items(void)
{
    if(j_state->struct_type != STRUCT_PULLDOWN)
    {
        json_error("Requires parent pulldown structure");
    }

    new_state_struct(0, STRUCT_MENU_ITEMS, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_m_item(void)
{
    if(j_state->struct_type != STRUCT_MENU_ITEMS)
    {
        json_error("requires parent menu-items array");
    }

    new_state_struct(sizeof(menu_item_t), STRUCT_MENU_ITEM, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_attribs(void)
{
    if((j_state->struct_type != STRUCT_BACKDROP) &&
       (j_state->struct_type != STRUCT_WINDOW) &&
       (j_state->struct_type != STRUCT_PULLDOWN) &&
       (j_state->struct_type != STRUCT_MENU_BAR))
    {
        json_error(
            "Requires parent backdrop, window, pulldown or menu bar");
    }

    new_state_struct(0, STRUCT_ATTRIBS, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_b_attribs(void)
{
    if((j_state->struct_type != STRUCT_BACKDROP) &&
       (j_state->struct_type != STRUCT_WINDOW))
    {
        json_error("Requires parent backdrop or window");
    }

    new_state_struct(0, STRUCT_B_ATTRIBS, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_s_attribs(void)
{
    if((j_state->struct_type != STRUCT_BACKDROP) &&
       (j_state->struct_type != STRUCT_WINDOW) &&
       (j_state->struct_type != STRUCT_PULLDOWN) &&
       (j_state->struct_type != STRUCT_MENU_BAR))
    {
        json_error(
            "Requires parent backdrop, window, pulldown or menu bar");
    }

    new_state_struct(0, STRUCT_S_ATTRIBS, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_d_attribs(void)
{
    if((j_state->struct_type != STRUCT_BACKDROP) &&
       (j_state->struct_type != STRUCT_WINDOW) &&
       (j_state->struct_type != STRUCT_PULLDOWN) &&
       (j_state->struct_type != STRUCT_MENU_BAR))
    {
        json_error(
            "Requires parent backdrop, window, pulldown or menu bar");
    }

    new_state_struct(0, STRUCT_D_ATTRIBS, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_rgb_fg(void)
{
    if((j_state->struct_type != STRUCT_ATTRIBS) &&
       (j_state->struct_type != STRUCT_B_ATTRIBS) &&
       (j_state->struct_type != STRUCT_S_ATTRIBS) &&
       (j_state->struct_type != STRUCT_D_ATTRIBS))
    {
        json_error("Requires parent atrribs structure");
    }

    new_state_struct(0, STRUCT_RGB_FG, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_rgb_bg(void)
{
    if((j_state->struct_type != STRUCT_ATTRIBS) &&
       (j_state->struct_type != STRUCT_B_ATTRIBS) &&
       (j_state->struct_type != STRUCT_S_ATTRIBS) &&
       (j_state->struct_type != STRUCT_D_ATTRIBS))
    {
        json_error("Requires parent atrribs structure");
    }

    new_state_struct(0, STRUCT_RGB_BG, STATE_L_BRACE);
}

// -----------------------------------------------------------------------

static void struct_flags(void)
{
    if((j_state->struct_type != STRUCT_BACKDROP) &&
       (j_state->struct_type != STRUCT_WINDOW) &&
       (j_state->struct_type != STRUCT_PULLDOWN) &&
       (j_state->struct_type != STRUCT_MENU_BAR))
    {
        json_error("Requires parent backdrop, window, pulldown or menu bar");
    }

    new_state_struct(0, STRUCT_FLAGS, STATE_L_BRACE);

    array = 1;  // tell state machine to expect a [ instead of {
}

// -----------------------------------------------------------------------

static void key_fg(void)
{
    if((j_state->struct_type != STRUCT_ATTRIBS) &&
       (j_state->struct_type != STRUCT_B_ATTRIBS) &&
       (j_state->struct_type != STRUCT_S_ATTRIBS) &&
       (j_state->struct_type != STRUCT_D_ATTRIBS))
    {
        json_error("Requires parent atrribs structure");
    }

    new_state_struct(0, KEY_FG, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_gray_fg(void)
{
    if((j_state->struct_type != STRUCT_ATTRIBS) &&
       (j_state->struct_type != STRUCT_B_ATTRIBS) &&
       (j_state->struct_type != STRUCT_S_ATTRIBS) &&
       (j_state->struct_type != STRUCT_D_ATTRIBS))
    {
        json_error("Requires parent atrribs structure");
    }

    new_state_struct(0, KEY_FG, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_bg(void)
{
    if((j_state->struct_type != STRUCT_ATTRIBS) &&
       (j_state->struct_type != STRUCT_B_ATTRIBS) &&
       (j_state->struct_type != STRUCT_S_ATTRIBS) &&
       (j_state->struct_type != STRUCT_D_ATTRIBS))
    {
        json_error("Requires parent atrribs structure");
    }

    // leaves state unchanged
    new_state_struct(0, KEY_BG, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_gray_bg(void)
{
    if((j_state->struct_type != STRUCT_ATTRIBS) &&
       (j_state->struct_type != STRUCT_B_ATTRIBS) &&
       (j_state->struct_type != STRUCT_S_ATTRIBS) &&
       (j_state->struct_type != STRUCT_D_ATTRIBS))
    {
        json_error("Requires parent atrribs structure");
    }

    // leaves state unchanged
    new_state_struct(0, KEY_GRAY_BG, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_red(void)
{
    ;
}

// -----------------------------------------------------------------------

static void key_green(void)
{
    ;
}

// -----------------------------------------------------------------------

static void key_blue(void)
{
    ;
}

// -----------------------------------------------------------------------

static void key_xco(void)
{
    if(j_state->struct_type != STRUCT_WINDOW)
    {
        json_error("Requires parent window");
    }
    new_state_struct(0, KEY_XCO, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_yco(void)
{
    if(j_state->struct_type != STRUCT_WINDOW)
    {
        json_error("Requires parent window");
    }
    new_state_struct(0, KEY_YCO, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_width(void)
{
    if(j_state->struct_type != STRUCT_WINDOW)
    {
        json_error("Requires parent window");
    }
    new_state_struct(0, KEY_WIDTH, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_height(void)
{
    if(j_state->struct_type != STRUCT_WINDOW)
    {
        json_error("Requires parent window");
    }
    new_state_struct(0, KEY_HEIGHT, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_name(void)
{
    if((j_state->struct_type != STRUCT_PULLDOWN) &&
       (j_state->struct_type != STRUCT_MENU_ITEM))
    {
        json_error("Requires parent pulldown or menu-item");
    }
    new_state_struct(0, KEY_NAME, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_flags(void)
{
    if(array != 0)
    {
        json_error("Nested arrays not allowed here");
    }

    if((j_state->struct_type != STRUCT_PULLDOWN) &&
       (j_state->struct_type != STRUCT_MENU_ITEM) &&
       (j_state->struct_type != STRUCT_WINDOW))
    {
        json_error("Requires parent window, pulldown or menu-item");
    }
    new_state_struct(0, KEY_FLAGS, STATE_VALUE);

    // tell state machine to require [ instead of {

    array = 1;
}

// -----------------------------------------------------------------------

static void key_border_type(void)
{
    if((j_state->struct_type != STRUCT_BACKDROP) &&
       (j_state->struct_type != STRUCT_WINDOW))
    {
        json_error("Requires parent window or backdrop");
    }
    new_state_struct(0, KEY_BORDER_TYPE, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_vector(void)
{
    if(j_state->struct_type != STRUCT_MENU_ITEM)
    {
        json_error("Requires parent menu-item");
    }
    new_state_struct(0, KEY_VECTOR, STATE_VALUE);
}

// -----------------------------------------------------------------------

static void key_shortcut(void)
{
    if(j_state->struct_type != STRUCT_MENU_ITEM)
    {
        json_error("Requires parent menu-item");
    }
    new_state_struct(0, KEY_SHORTCUT, STATE_VALUE);
}

// -----------------------------------------------------------------------

static const switch_t key_types[] =
{
    { 0x6b77251c,  key_fg          },
    { 0xaa3b6788,  key_gray_fg     },
    { 0x6f772ba0,  key_bg          },
    { 0xa63b61c4,  key_gray_bg     },
    { 0x3a72d292,  key_red         },
    { 0xf73297b2,  key_green       },
    { 0x4f068569,  key_blue        },
    { 0x1c63995d,  key_xco         },
    { 0x3461800c,  key_yco         },
    { 0x182e64eb,  key_width       },
    { 0x4c47d5c0,  key_height      },
    { 0x2f8b3bf4,  key_name        },
    { 0x68cdf632,  key_flags       },
    { 0x362bb2fc,  key_border_type },
    { 0x0ee694b4,  key_vector      },
    { 0x1c13e01f,  key_shortcut    },
};

#define NUM_KEYS (sizeof(key_types) / sizeof(key_types[0]))

// -----------------------------------------------------------------------

static const switch_t object_types[] =
{
    { 0x2ff97421,  struct_screen     },
    { 0x1025ba8c,  struct_windows    },
    { 0x8ae7f465,  struct_window     },
    { 0x3bacc0d7,  struct_backdrop   },
    { 0x95fe0788,  struct_m_bar      },
    { 0x80f84daf,  struct_pulldowns  },
    { 0x09159434,  struct_pulldown   },
    { 0x196fe4d3,  struct_m_items    },
    { 0x90f9ece0,  struct_m_item     },
    { 0xbc6bca20,  struct_attribs    },
    { 0x77d19b03,  struct_b_attribs  },
    { 0x4d8ce0ce,  struct_s_attribs  },
    { 0x19007641,  struct_d_attribs  },
    { 0xea8606c2,  struct_rgb_fg     },
    { 0xe686003e,  struct_rgb_bg     },
    { 0x68cdf632,  struct_flags      }
};

#define NUM_OBJECTS (sizeof(object_types) / sizeof(object_types[0]))

// -----------------------------------------------------------------------
// a key is a "quoted-name" used to name objects and key values

static void state_key(void)
{
    uint16_t i;
    int f;

    size_t len = strlen(json_token);

    if((json_token[0]       != '"') &&
       (json_token[len - 1] != '"'))
    {
        json_error("Key names must be quoted");
    }

    // strip qutes from token and recalculate hash
    for(i = 0; i < len - 2; i++)
    {
        json_token[i] = json_token[i + 1];
    }
    json_token[i] = '\0';
    json_hash = fnv_hash(json_token);

    // objects are a type of key which are a container for keys
    f = re_switch(object_types, NUM_OBJECTS, json_hash);

    // if reswitch returned -1 here then we did not just parse
    // in an object name but a possible key name

    if(f == -1)
    {
        f = re_switch(key_types, NUM_KEYS, json_hash);
        if(f == -1)
        {
            json_error("Unknown key name");
        }
    }

    // every key must be followed by a colon

    token();
    json_hash = fnv_hash(json_token);

    if(json_hash != json_syntax[JSON_COLON])
    {
        json_error("Missing colon");
    }
}

// -----------------------------------------------------------------------

static void value_fg(void)
{
    j_state_t *parent  = j_state->parent;
    j_state_t *gparent = parent->parent;

    void *structure   = gparent->structure;
    uint32_t ptype    = parent->struct_type;
    uint32_t gtype    = gparent->struct_type;

    if(key_value > 15)
    {
        json_error("Value out of range");
    }

    switch(ptype)
    {
        case STRUCT_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->normal[FG] = key_value;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->normal[FG] = key_value;
                    break;
                case STRUCT_WINDOW:
                    ((window_t *)structure)->attrs[FG] = key_value;
                    break;
            }
            break;

        case STRUCT_B_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->attr[FG] = key_value;
                    break;
                case STRUCT_WINDOW:
                    ((window_t *)structure)->attrs[FG] = key_value;
                    break;
            }
            break;

        case STRUCT_S_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->selected[FG] = key_value;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->selected[FG] = key_value;
                    break;
            }
            break;

        case STRUCT_D_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->disabled[FG] = key_value;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->disabled[FG] = key_value;
                    break;
            }
            break;
    }
}

// -----------------------------------------------------------------------

static void value_gray_fg(void)
{
    j_state_t *parent  = j_state->parent;
    j_state_t *gparent = parent->parent;

    void *structure   = gparent->structure;
    uint32_t ptype    = parent->struct_type;
    uint32_t gtype    = gparent->struct_type;

    if(key_value > 23)
    {
        json_error("Value out of range");
    }

    switch(ptype)
    {
        case STRUCT_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->normal[FG] = key_value;
                    ((pulldown_t *)structure)->normal[ATTR] |= FG_GRAY;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->normal[FG] = key_value;
                    ((menu_bar_t *)structure)->normal[ATTR] |= FG_GRAY;
                    break;
                case STRUCT_WINDOW:
                    ((window_t *)structure)->attrs[FG] = key_value;
                    ((window_t *)structure)->attrs[ATTR] |= FG_GRAY;
                    break;
            }
            break;

        case STRUCT_B_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->attr[FG] = key_value;
                    ((pulldown_t *)structure)->attr[ATTR] |= FG_GRAY;
                    break;
                case STRUCT_WINDOW:
                    ((window_t *)structure)->bdr_attrs[FG] = key_value;
                    ((window_t *)structure)->bdr_attrs[FG] = key_value;
                    break;
            }
            break;

        case STRUCT_S_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->selected[FG] = key_value;
                    ((pulldown_t *)structure)->selected[ATTR] |= FG_GRAY;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->selected[FG] = key_value;
                    ((menu_bar_t *)structure)->selected[ATTR] |= FG_GRAY;
                    break;
            }
            break;

        case STRUCT_D_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->disabled[FG] = key_value;
                    ((pulldown_t *)structure)->disabled[ATTR] |= FG_GRAY;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->disabled[FG] = key_value;
                    ((menu_bar_t *)structure)->disabled[ATTR] |= FG_GRAY;
                    break;
            }
            break;
    }
}

// -----------------------------------------------------------------------

static void value_bg(void)
{
    j_state_t *parent  = j_state->parent;
    j_state_t *gparent = parent->parent;

    void *structure   = gparent->structure;
    uint32_t ptype    = parent->struct_type;
    uint32_t gtype    = gparent->struct_type;

    if(key_value > 15)
    {
        json_error("Value out of range");
    }

    switch(ptype)
    {
        case STRUCT_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->normal[BG] = key_value;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->normal[BG] = key_value;
                    break;
                case STRUCT_WINDOW:
                    ((window_t *)structure)->attrs[BG] = key_value;
                    break;
            }
            break;

        case STRUCT_B_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->attr[BG] = key_value;
                    break;
                case STRUCT_WINDOW:
                    ((window_t *)structure)->bdr_attrs[BG] = key_value;
                    break;
            }
            break;

        case STRUCT_S_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->selected[BG] = key_value;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->selected[BG] = key_value;
                    break;
            }
            break;

        case STRUCT_D_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->disabled[BG] = key_value;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->disabled[BG] = key_value;
                    break;
            }
            break;
    }
}

// -----------------------------------------------------------------------

static void value_gray_bg(void)
{
    j_state_t *parent  = j_state->parent;
    j_state_t *gparent = parent->parent;

    void *structure   = gparent->structure;
    uint32_t ptype    = parent->struct_type;
    uint32_t gtype    = gparent->struct_type;

    if(key_value > 23)
    {
        json_error("Value out of range");
    }

    switch(ptype)
    {
        case STRUCT_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->normal[BG] = key_value;
                    ((pulldown_t *)structure)->normal[ATTR] |= BG_GRAY;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->normal[BG] = key_value;
                    ((menu_bar_t *)structure)->normal[ATTR] |= BG_GRAY;
                    break;
                case STRUCT_WINDOW:
                    ((window_t *)structure)->attrs[BG] = key_value;
                    ((window_t *)structure)->attrs[ATTR] |= BG_GRAY;
                    break;
            }
            break;

        case STRUCT_B_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->attr[BG] = key_value;
                    ((pulldown_t *)structure)->attr[ATTR] |= BG_GRAY;
                    break;
                case STRUCT_WINDOW:
                    ((window_t *)structure)->bdr_attrs[BG] = key_value;
                    ((window_t *)structure)->bdr_attrs[ATTR] |= BG_GRAY;
                    break;
            }
            break;

        case STRUCT_S_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->selected[BG] = key_value;
                    ((pulldown_t *)structure)->selected[ATTR] |= BG_GRAY;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->selected[BG] = key_value;
                    ((menu_bar_t *)structure)->selected[ATTR] |= BG_GRAY;
                    break;
            }
            break;

        case STRUCT_D_ATTRIBS:
            switch(gtype)
            {
                case STRUCT_PULLDOWN:
                    ((pulldown_t *)structure)->disabled[BG] = key_value;
                    ((pulldown_t *)structure)->disabled[ATTR] |= BG_GRAY;
                    break;
                case STRUCT_MENU_BAR:
                    ((menu_bar_t *)structure)->disabled[BG] = key_value;
                    ((menu_bar_t *)structure)->disabled[ATTR] |= BG_GRAY;
                    break;
            }
            break;
    }
}

// -----------------------------------------------------------------------

static void value_red(void)
{
    ;
}

// -----------------------------------------------------------------------

static void value_green(void)
{
    ;
}

// -----------------------------------------------------------------------

static void value_blue(void)
{
    ;
}

// -----------------------------------------------------------------------

static void value_xco(void)
{
    j_state_t *parent  = j_state->parent;
    window_t *win = (window_t *)parent->structure;

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
    window_t *win = (window_t *)parent->structure;

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
    window_t *win = (window_t *)parent->structure;

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
    window_t *win = (window_t *)parent->structure;

    if(key_value > console_height)
    {
        json_error("Window too wide to fit in console");
    }
    win->height = key_value;
}

// -----------------------------------------------------------------------

static void value_name(void)
{
    ;
}

// -----------------------------------------------------------------------

static void value_flags(void)
{
    ;
}

// -----------------------------------------------------------------------

static void value_border_type(void)
{
    ;
}

// -----------------------------------------------------------------------

static void value_vector(void)
{
    ;
}

// -----------------------------------------------------------------------

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
    { 0x68cdf632,  value_flags       },
    { 0x362bb2fc,  value_border_type },
    { 0x0ee694b4,  value_vector      },
    { 0x1c13e01f,  value_shortcut    },
};

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

static uint32_t parse_number(void)
{
    return 1;
}

// -----------------------------------------------------------------------

static void state_value(void)
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

    if(key_value == -1)
    {
        // could be a quoted string too so NAN is possible
        key_value = parse_number();
    }

    if(key_value == -1)
    {
        json_error("Unknown value");
    }

    // we got this far, one of these will work
    re_switch(value_types, NUM_KEYS, json_hash);

    j_state->state = (has_comma != 0)
        ? STATE_KEY
        : STATE_R_BRACE ;
}

// -----------------------------------------------------------------------

static void state_r_brace(void)
{
    uint16_t has_comma = 0;
    uint16_t end = strlen(json_token) - 1;

    if(array == 2)
    {
        if(json_hash != json_syntax[JSON_R_BRACKET])
        {
            json_error("Expected closing bracket");
        }
        // we got the ] sp now we need the } this state
        // expects normally
        array = 0;
        token();
        json_hash = fnv_hash(json_token);
    }

    if(json_token[end] == ',')
    {
        json_token[end] = '\0';
        json_hash = fnv_hash(json_token);
        has_comma = 1;
    }

    if(json_hash != json_syntax[JSON_R_BRACE])
    {
        json_error("Closing brace missing");
    }

    if(j_stack.count != 0)
    {
        j_pop();

        j_state->state = (has_comma == 0)
            ? j_state->state + 1
            : STATE_KEY;
     }
    else
    {
        // if has comma json_error("id10t") ?
        j_state->state = STATE_DONE;
    }
}

// -----------------------------------------------------------------------

static const switch_t states[] =
{
    { STATE_L_BRACE,  state_l_brace },
    { STATE_KEY,      state_key     },
    { STATE_VALUE,    state_value   },
    { STATE_R_BRACE,  state_r_brace },
};

// -----------------------------------------------------------------------

void json_state_machine(char *json, size_t len)
{
    j_state = j_alloc(sizeof(*j_state));

    json_data = json;
    json_len  = len;

    j_state->struct_type = -1;
    j_state->state = JSON_L_BRACE;

    json_de_tab(json, len);

    while(j_state->state != STATE_DONE)
    {
        token();
        json_hash = fnv_hash(json_token);

        re_switch(states, NUM_STATES, j_state->state);
    }
    // TODO
    // allocate screen and window structures backing stores here
    // because we know there sizes now
}

// =======================================================================
