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
uint32_t name_hash;
uint32_t has_comma;     // true if last token had a comma on it
uint16_t array;         // true if structure is an array

static list_t j_stack;

// -----------------------------------------------------------------------

typedef enum
{
    STATE_STRUCT_COLON,
    STATE_L_BRACE,
    STATE_STUCT_NAME,
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
    STRUCT_FLAGS
} struct_type_t;

// -----------------------------------------------------------------------

j_state_t *j_state;

// -----------------------------------------------------------------------

static void j_push(void)
{
    list_append_node(&j_stack, j_state);
}

__attribute__((used))
static void j_pop(void)
{
    j_state = list_pop(&j_stack);
}

// -----------------------------------------------------------------------

void json_error(char *s)
{
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
// json files and indiidual structures must start with a {

static void state_l_brace(void)
{
    if(json_hash != json_syntax[JSON_L_BRACE])
    {
        json_error("Opening brace missing");
    }

    // are we expecting to see an opening bracket after the {
    if(array == 1)
    {
        token();
        if(json_hash == json_syntax[JSON_L_BRACKET])
        {
            array = 2;
        }
        else
        {
            json_error("Expected opening bracket");
        }
    }
    j_state->state++;
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

    scr = j_alloc(sizeof(*scr));

    j_state->parent      = NULL;
    j_state->structure   = scr;
    j_state->struct_type = STRUCT_SCREEN;
    j_state->state       = STATE_STRUCT_COLON;
}

// -----------------------------------------------------------------------

static void struct_windows(void)
{
    j_state_t *j;

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_WINDOWS;
    j->state       = STATE_STRUCT_COLON;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------
// for there to be a window there must first be a screen

static void struct_window(void)
{
    window_t *win;
    j_state_t *j;

    /// *** ensure parent is struct_windows

    j = j_alloc(sizeof(*j));
    win = j_alloc(sizeof(*win));

    j->parent      = j_state;
    j->structure   = win;
    j->struct_type = STRUCT_WINDOW;
    j->state       = STATE_STRUCT_COLON;

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_backdrop(void)
{
    window_t *win;
    j_state_t *j;

// ensure parent structure is a screen

    j = j_alloc(sizeof(*j));
    win = j_alloc(sizeof(*win));

    j->parent      = j_state;
    j->structure   = win;
    j->struct_type = STRUCT_BACKDROP;
    j->state       = STATE_STRUCT_COLON;

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_m_bar(void)
{
    menu_bar_t *bar;
    j_state_t *j;

    // ensure parent structure is a screen

    j = j_alloc(sizeof(*j));
    bar = j_alloc(sizeof(*bar));

    j->parent      = j_state;
    j->structure   = bar;
    j->struct_type = STRUCT_MENU_BAR;
    j->state       = STATE_STRUCT_COLON;

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_pulldowns(void)
{
    // ensure parent structure is a menu bar

    j_state_t *j;

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_PULLDOWNS;
    j->state = 0;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_pulldown(void)
{
    pulldown_t *pd;
    j_state_t *j;

    // ensure parent structure is a pulldowns

    j = j_alloc(sizeof(*j));
    pd = j_alloc(sizeof(*pd));

    j->parent      = j_state;
    j->structure   = pd;
    j->struct_type = STRUCT_PULLDOWN;
    j->state = 0;

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_m_items(void)
{
    j_state_t *j;

    // ensure parent structure is a pulldown

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_MENU_ITEMS;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_m_item(void)
{
    menu_item_t *item;
    j_state_t *j;

    // ensure parent is a pulldown

    j = j_alloc(sizeof(*j));
    item = j_alloc(sizeof(*item));

    j->parent      = j_state;
    j->structure   = item;
    j->struct_type = STRUCT_MENU_ITEM;

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_attribs(void)
{
    j_state_t *j;

    // parent structure can be a backdrop, a window, a pulldown
    // or a menu bar

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_ATTRIBS;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_b_attribs(void)
{
    j_state_t *j;

    // parent structure can be a backdrop, a window, a pulldown
    // or a menu bar

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_B_ATTRIBS;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_s_attribs(void)
{
    j_state_t *j;

    // parent structure can be a backdrop, a window, a pulldown
    // or a menu bar

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_S_ATTRIBS;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_d_attribs(void)
{
    j_state_t *j;

    // parent structure can be a backdrop, a window, a pulldown
    // or a menu bar

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_D_ATTRIBS;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_rgb_fg(void)
{
    j_state_t *j;

    // parent structure can be a backdrop, a window, a pulldown
    // or a menu bar

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_RGB_FG;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_rgb_bg(void)
{
    j_state_t *j;

    // parent structure can be a backdrop, a window, a pulldown
    // or a menu bar

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_RGB_BG;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static void struct_flags(void)
{
    j_state_t *j;

    // parent structure can be a backdrop, a window, a pulldown
    // or a menu bar

    j = j_alloc(sizeof(*j));

    j->parent      = j_state;
    j->structure   = NULL;
    j->struct_type = STRUCT_FLAGS;

    array = 1;  // tell state machine to expect a [ after the {

    j_push();
    j_state = j;
}

// -----------------------------------------------------------------------

static const switch_t struct_type[] =
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

#define NUM_STRUCTS (sizeof(struct_type) / sizeof(struct_type[0]))

// -----------------------------------------------------------------------
// quoted name of structure to store value to

static void state_struct(void)
{
    uint16_t i;
    size_t len = strlen(json_token);

    if((json_token[0]       != '"') &&
       (json_token[len - 1] != '"'))
    {
        json_error("Field names must be quoted");
    }

    // strip qutes from token and recalculate hash
    for(i = 0; i < len - 2; i++)
    {
        json_token[i] = json_token[i + 1];
    }
    json_token[i] = '0';

    json_hash = fnv_hash(json_token);

    // if reswitch returns -1 here then we did not just parse in a
    // structure name but a structure element name so....
    // we are just trying to set some structure element to a value.

    name_hash = (re_switch(struct_type, NUM_STRUCTS, json_hash) == -1)
        ? json_hash : 0;
}

// -----------------------------------------------------------------------

static void state_colon(void)
{
    if(json_hash != json_syntax[JSON_COLON])
    {
        json_error("Colon missing");
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

    free(j_state->structure);
    free(j_state);

    j_state = list_pop(&j_stack);
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
    { STATE_STUCT_NAME,   state_struct  },

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
