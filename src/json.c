// parsing json definitions for user interface layout
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

char json_token[TOKEN_LEN];    // space delimited token extracted from data
uint32_t json_hash;
uint32_t has_comma;     // true if last token had a comma on it
uint16_t array;         // true if structure is an array

static list_t j_stack;

// -----------------------------------------------------------------------

typedef enum
{
    STATE_STRUCT_COLON,
    STATE_L_BRACE,
    STATE_STRUCT_NAME,
    STATE_VALUE_COLON,
    STATE_VALUE,
    STATE_R_BRACE,
    STATE_DONE,
    NUM_STATES
} json_state_t;

// -----------------------------------------------------------------------
// json syntax

const uint32_t json_syntax[] =
{
    0x050c5d3c,             // #
    0x050c5d25,             // :
    0x050c5d64,             // {
    0x050c5d62,             // }
    0x050c5d44,             // [
    0x050c5d42              // ]
};

// pfffsets into above array of hash values
typedef enum
{
    JSON_COMMENT,
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
    KEY_XCO,
    KEY_YCO,
    KEY_WIDTH,
    KEY_HEIGHT,
    KEY_NAME,
    KEY_FLAGS,
    KEY_BORDER_TYPE,
    KEY_VECTOR,
    KEY_SHORTCUT
} struct_type_t;

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
    free(j_state->structure);
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
            json_error("Array needs opening bracket");
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
    screen_t *scr;

    if(j_stack.count != 0)
    {
        json_error("There can be only one screen");
    }

    // this was a temp j_state just to get us to here
    // the screen is the foundation for the entire UI

    free(j_state);
    j_state = NULL;

    new_state_struct(sizeof(*scr), STRUCT_SCREEN, STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------

static void struct_windows(void)
{
    if(j_state->struct_type != STRUCT_SCREEN)
    {
        json_error("Requires parent screen");
    }

    new_state_struct(0, STRUCT_WINDOWS, STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------
// for there to be a window there must first be a screen

static void struct_window(void)
{
    if(j_state->struct_type != STRUCT_WINDOWS)
    {
        json_error("Requires parent windows structure");
    }

    new_state_struct(sizeof(window_t), STRUCT_WINDOW,
        STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------

static void struct_backdrop(void)
{
    if(j_state->struct_type != STRUCT_SCREEN)
    {
        json_error("Requires parent screen");
    }

    new_state_struct(sizeof(window_t), STRUCT_BACKDROP,
        STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------

static void struct_m_bar(void)
{
    if(j_state->struct_type != STRUCT_SCREEN)
    {
        json_error("Requires parent screen");
    }

    new_state_struct(sizeof(menu_bar_t), STRUCT_MENU_BAR,
        STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------

static void struct_pulldowns(void)
{
    if(j_state->struct_type != STRUCT_MENU_BAR)
    {
        json_error("Requires parent screen");
    }

    new_state_struct(0, STRUCT_PULLDOWNS, STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------

static void struct_pulldown(void)
{
    if(j_state->struct_type != STRUCT_PULLDOWNS)
    {
        json_error("Requires parent pulldowns structure");
    }

    new_state_struct(sizeof(pulldown_t), STRUCT_PULLDOWN,
        STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------

static void struct_m_items(void)
{
    if(j_state->struct_type != STRUCT_PULLDOWN)
    {
        json_error("Requires parent pulldown structure");
    }

    new_state_struct(0, STRUCT_MENU_ITEMS, STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------

static void struct_m_item(void)
{
    if(j_state->struct_type != STRUCT_MENU_ITEMS)
    {
        json_error("requires parent menu-items array");
    }

    new_state_struct(sizeof(menu_item_t), STRUCT_MENU_ITEM,
        STATE_STRUCT_COLON);
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

    new_state_struct(0, STRUCT_ATTRIBS, STATE_STRUCT_COLON);
}

// -----------------------------------------------------------------------

static void struct_b_attribs(void)
{
    if((j_state->struct_type != STRUCT_BACKDROP) &&
       (j_state->struct_type != STRUCT_WINDOW))
    {
        json_error("Requires parent backdrop or window");
    }

    new_state_struct(0, STRUCT_B_ATTRIBS, STATE_STRUCT_COLON);
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

    new_state_struct(0, STRUCT_S_ATTRIBS, STATE_STRUCT_COLON);
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

    new_state_struct(0, STRUCT_D_ATTRIBS, STATE_STRUCT_COLON);
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

    new_state_struct(0, STRUCT_RGB_FG, STATE_STRUCT_COLON);
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

    new_state_struct(0, STRUCT_RGB_BG, STATE_STRUCT_COLON);
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

    new_state_struct(0, STRUCT_FLAGS, STATE_STRUCT_COLON);

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

    // leaves state unchanged
    new_state_struct(0, KEY_FG, j_state->state);
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

    // leaves state unchanged
    new_state_struct(0, KEY_FG, j_state->state);
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
    new_state_struct(0, KEY_BG, j_state->state);
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
    new_state_struct(0, KEY_GRAY_BG, j_state->state);
}

// -----------------------------------------------------------------------

static void key_xco(void)
{
    if(j_state->struct_type != STRUCT_WINDOW)
    {
        json_error("Requires parent window");
    }
    new_state_struct(0, KEY_XCO, j_state->state);
}

// -----------------------------------------------------------------------

static void key_yco(void)
{
    if(j_state->struct_type != STRUCT_WINDOW)
    {
        json_error("Requires parent window");
    }
    new_state_struct(0, KEY_YCO, j_state->state);
}

// -----------------------------------------------------------------------

static void key_width(void)
{
    if(j_state->struct_type != STRUCT_WINDOW)
    {
        json_error("Requires parent window");
    }
    new_state_struct(0, KEY_WIDTH, j_state->state);
}

// -----------------------------------------------------------------------

static void key_height(void)
{
    if(j_state->struct_type != STRUCT_WINDOW)
    {
        json_error("Requires parent window");
    }
    new_state_struct(0, KEY_HEIGHT, j_state->state);
}

// -----------------------------------------------------------------------

static void key_name(void)
{
    if((j_state->struct_type != STRUCT_PULLDOWN) &&
       (j_state->struct_type != STRUCT_MENU_ITEM))
    {
        json_error("Requires parent pulldown or menu-item");
    }
    new_state_struct(0, KEY_NAME, j_state->state);
}

// -----------------------------------------------------------------------

static void key_flags(void)
{
    if((j_state->struct_type != STRUCT_PULLDOWN) &&
       (j_state->struct_type != STRUCT_MENU_ITEM) &&
       (j_state->struct_type != STRUCT_WINDOW))
    {
        json_error("Requires parent window, pulldown or menu-item");
    }
    new_state_struct(0, KEY_FLAGS, j_state->state);

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
    new_state_struct(0, KEY_BORDER_TYPE, j_state->state);
}

// -----------------------------------------------------------------------

static void key_vector(void)
{
    if(j_state->struct_type != STRUCT_MENU_ITEM)
    {
        json_error("Requires parent menu-item");
    }
    new_state_struct(0, KEY_VECTOR, j_state->state);
}

// -----------------------------------------------------------------------

static void key_shortcut(void)
{
    if(j_state->struct_type != STRUCT_MENU_ITEM)
    {
        json_error("Requires parent menu-item");
    }
    new_state_struct(0, KEY_SHORTCUT, j_state->state);
}

// -----------------------------------------------------------------------

static const switch_t key_types[] =
{
    { 0x6b77251c,  key_fg          },
    { 0xaa3b6788,  key_gray_fg     },
    { 0x6f772ba0,  key_bg          },
    { 0xa63b61c4,  key_gray_bg     },
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

static const switch_t struct_types[] =
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

#define NUM_STRUCTS (sizeof(struct_types) / sizeof(struct_types[0]))

// -----------------------------------------------------------------------

static void state_struct(void)
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


    f = re_switch(struct_types, NUM_STRUCTS, json_hash);

    // if reswitch returned -1 here then we did not just parse
    // in a structure name but a possible key name so....
    // we are just trying to set some key to some value.

    if(f == -1)
    {
        f = re_switch(key_types, NUM_KEYS, json_hash);
        if(f == -1)
        {
            json_error("Unknown key name");
        }
        j_state->state++;
    }
}

// -----------------------------------------------------------------------

static void state_colon(void)
{
    if(json_hash != json_syntax[JSON_COLON])
    {
        json_error("Missing Colon");
    }
    j_state->state++;
}

// -----------------------------------------------------------------------

static void state_value(void)
{
    uint16_t len = strlen(json_token) - 1;

    if(json_token[len -1] == ',')
    {
        len--;
        json_token[len] = '\0';
        json_hash = fnv_hash(json_token);
    }

    // ... lots of logic to go here etc

}

// -----------------------------------------------------------------------

static void state_r_brace(void)
{
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

    if(json_hash != json_syntax[JSON_R_BRACE])
    {
        json_error("Closing brace missing");
    }

    j_pop();
    j_state->state++;

    if(j_stack.count == 0)
    {
        j_state->state = STATE_DONE;
    }
}

// -----------------------------------------------------------------------

static const switch_t states[] =
{
    { STATE_STRUCT_COLON, state_colon   },
    { STATE_L_BRACE,      state_l_brace },
    { STATE_STRUCT_NAME,  state_struct  },

    { STATE_VALUE_COLON,  state_colon   },
    { STATE_VALUE,        state_value   },
    { STATE_R_BRACE,      state_r_brace }
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
}

// =======================================================================
