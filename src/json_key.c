// json_key.c   json key name parsing
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/list.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

extern list_t j_stack;
extern j_state_t *j_state;
extern uint32_t json_hash;
extern const uint32_t json_syntax[];
extern screen_t *active_screen;

// -----------------------------------------------------------------------
// were only referencing this to get its size

extern uint8_t attrs[8];
extern char json_token[TOKEN_LEN];
extern uint16_t console_width;
extern uint16_t console_height;

// -----------------------------------------------------------------------
// must be the first object specified

static void struct_screen(void)
{
    screen_t *scr;

    if(j_stack.count == 0)
    {
        json_new_state_struct(sizeof(screen_t), STRUCT_SCREEN);
        scr = j_state->structure;

        // you have no say in this
        scr->width  = console_width;
        scr->height = console_height;

        active_screen = scr;
        return;
    }

    json_error("There can be only one!");
}

// -----------------------------------------------------------------------

static void struct_windows(void)
{
    if(j_state->struct_type == STRUCT_SCREEN)
    {
        json_new_state_struct(0, STRUCT_WINDOWS);
        return;
    }

    json_error("Requires parent screen");
}

// -----------------------------------------------------------------------
// for there to be a window there must first be a screen

static void struct_window(void)
{
    if(j_state->struct_type == STRUCT_WINDOWS)
    {
        json_new_state_struct(sizeof(window_t), STRUCT_WINDOW);
        return;
    }

    json_error("Requires parent windows structure");
}

// -----------------------------------------------------------------------
// special non writeable window (well you can but usually dont :)

static void struct_backdrop(void)
{
    if(j_state->struct_type == STRUCT_SCREEN)
    {
        json_new_state_struct(sizeof(window_t), STRUCT_BACKDROP);
        return;
    }

    json_error("Requires parent screen");
}

// -----------------------------------------------------------------------

static void struct_m_bar(void)
{
    if(j_state->struct_type == STRUCT_SCREEN)
    {
        json_new_state_struct(sizeof(menu_bar_t), STRUCT_MENU_BAR);
        return;
    }

    json_error("Requires parent screen");
}

// -----------------------------------------------------------------------

static void struct_pulldowns(void)
{
    if(j_state->struct_type == STRUCT_MENU_BAR)
    {
        // there is no actual structure for this, as this item is
        // populated through json this state_t's parent structure
        // is what will actually get populated

        json_new_state_struct(0, STRUCT_PULLDOWNS);
        return;
    }

    json_error("Requires parent menu-bar");
}

// -----------------------------------------------------------------------

static void struct_pulldown(void)
{
    if(j_state->struct_type == STRUCT_PULLDOWNS)
    {
        json_new_state_struct(sizeof(pulldown_t), STRUCT_PULLDOWN);
        return;
    }

    json_error("Requires parent pulldowns structure");
}

// -----------------------------------------------------------------------

static void struct_m_items(void)
{
    if(j_state->struct_type == STRUCT_PULLDOWN)
    {
        // there is no actual structure for this, as this item is
        // populated through json this state_t's parent structure
        // is what will actually get populated

        json_new_state_struct(0, STRUCT_MENU_ITEMS);
        return;
    }

    json_error("Requires parent pulldown structure");
}

// -----------------------------------------------------------------------

static void struct_m_item(void)
{
    if(j_state->struct_type == STRUCT_MENU_ITEMS)
    {
        json_new_state_struct(sizeof(menu_item_t), STRUCT_MENU_ITEM);
        return;
    }

    json_error("Requires parent menu-items");
}

// -----------------------------------------------------------------------

static void struct_attribs(void)
{
    if((j_state->struct_type == STRUCT_BACKDROP) ||
       (j_state->struct_type == STRUCT_WINDOW)   ||
       (j_state->struct_type == STRUCT_PULLDOWN) ||
       (j_state->struct_type == STRUCT_MENU_BAR))
    {
        json_new_state_struct(sizeof(attrs), STRUCT_ATTRIBS);
        return;
    }

    json_error(
        "Requires parent backdrop, window, pulldown or menu-bar");
}

// -----------------------------------------------------------------------

static void struct_b_attribs(void)
{
    if((j_state->struct_type == STRUCT_BACKDROP) ||
       (j_state->struct_type == STRUCT_WINDOW))
    {
        json_new_state_struct(sizeof(attrs), STRUCT_B_ATTRIBS);
        return;
    }

    json_error("Requires parent backdrop or window");
}

// -----------------------------------------------------------------------

static void struct_s_attribs(void)
{
    if((j_state->struct_type == STRUCT_PULLDOWN) ||
       (j_state->struct_type == STRUCT_MENU_BAR))
    {
        json_new_state_struct(sizeof(attrs), STRUCT_S_ATTRIBS);
        return;
    }

    json_error(
        "Requires parent pulldown or menu-bar");
}

// -----------------------------------------------------------------------

static void struct_d_attribs(void)
{
    if((j_state->struct_type == STRUCT_PULLDOWN) ||
       (j_state->struct_type == STRUCT_MENU_BAR))
    {
        json_new_state_struct(sizeof(attrs), STRUCT_D_ATTRIBS);
        return;
    }

    json_error(
        "Requires parent pulldown or menu bar");
}

// -----------------------------------------------------------------------

static void struct_rgb_fg(void)
{
    if((j_state->struct_type == STRUCT_ATTRIBS)   ||
       (j_state->struct_type == STRUCT_B_ATTRIBS) ||
       (j_state->struct_type == STRUCT_S_ATTRIBS) ||
       (j_state->struct_type == STRUCT_D_ATTRIBS))
    {
        // there is no actual structure for this, as this item is
        // populated through json this state_t's parent structure
        // is what will actually get populated

        json_new_state_struct(0, STRUCT_RGB_FG);
        return;
    }

    json_error("Requires parent atrribs structure");
}

// -----------------------------------------------------------------------

static void struct_rgb_bg(void)
{
    if((j_state->struct_type == STRUCT_ATTRIBS)   ||
       (j_state->struct_type == STRUCT_B_ATTRIBS) ||
       (j_state->struct_type == STRUCT_S_ATTRIBS) ||
       (j_state->struct_type == STRUCT_D_ATTRIBS))
    {
        // there is no actual structure for this, as this item is
        // populated through json this state_t's parent structure
        // is what will actually get populated

        json_new_state_struct(0, STRUCT_RGB_BG);
        return;
    }

    json_error("Requires parent atrribs structure");
}

// -----------------------------------------------------------------------

static void struct_flags(void)
{
    if((j_state->struct_type == STRUCT_BACKDROP) ||
       (j_state->struct_type == STRUCT_WINDOW)   ||
       (j_state->struct_type == STRUCT_PULLDOWN) ||
       (j_state->struct_type == STRUCT_MENU_BAR))
    {
        // there is no actual structure for this, as this item is
        // populated through json this state_t's parent structure
        // is what will actually get populated

        json_new_state_struct(0, STRUCT_FLAGS);
        return;
    }

    json_error("Requires parent backdrop, window, pulldown or menu-bar");
}

// -----------------------------------------------------------------------

static void key_attr(uint16_t key)
{
    if((j_state->struct_type == STRUCT_ATTRIBS)   ||
       (j_state->struct_type == STRUCT_B_ATTRIBS) ||
       (j_state->struct_type == STRUCT_S_ATTRIBS) ||
       (j_state->struct_type == STRUCT_D_ATTRIBS))
    {
        json_new_state_struct(0, key);
        return;
    }

    json_error("Requires parent atrribs structure");
}

// -----------------------------------------------------------------------

static void key_fg(void)        { key_attr(KEY_FG);      }
static void key_bg(void)        { key_attr(KEY_BG);      }
static void key_gray_fg(void)   { key_attr(KEY_GRAY_FG); }
static void key_gray_bg(void)   { key_attr(KEY_GRAY_BG); }

// -----------------------------------------------------------------------

static void key_rgb(uint16_t key)
{
    if((j_state->struct_type == STRUCT_RGB_FG) ||
       (j_state->struct_type == STRUCT_RGB_BG))
    {
        json_new_state_struct(0, key);
        return;
    }

    json_error("Requires parent RGB");
}

// -----------------------------------------------------------------------

static void key_red(void)    { key_rgb(KEY_RED);   }
static void key_green(void)  { key_rgb(KEY_GREEN); }
static void key_blue(void)   { key_rgb(KEY_BLUE);  }

// -----------------------------------------------------------------------

static void key_xywh(uint16_t key)
{
    if(j_state->struct_type == STRUCT_WINDOW)
    {
        json_new_state_struct(0, key);
        return;
    }

    json_error("Requires parent window");
}

// -----------------------------------------------------------------------

static void key_xco(void)    { key_xywh(KEY_XCO);    }
static void key_yco(void)    { key_xywh(KEY_YCO);    }
static void key_width(void)  { key_xywh(KEY_WIDTH);  }
static void key_height(void) { key_xywh(KEY_HEIGHT); }

// -----------------------------------------------------------------------

static void key_name(void)
{
    if((j_state->struct_type == STRUCT_PULLDOWN)   ||
       (j_state->struct_type == STRUCT_MENU_ITEM))
    {
        json_new_state_struct(0, KEY_NAME);
        return;
    }

    json_error("Requires parent pulldown or menu-item");
}

// -----------------------------------------------------------------------

static void key_flags(void)
{
    if((j_state->struct_type == STRUCT_PULLDOWN)  ||
       (j_state->struct_type == STRUCT_MENU_ITEM) ||
       (j_state->struct_type == STRUCT_WINDOW))
    {
        json_new_state_struct(0, KEY_FLAGS);
        return;
    }

    json_error("Requires parent window, pulldown or menu-item");
}

// -----------------------------------------------------------------------

static void key_border_type(void)
{
    if((j_state->struct_type == STRUCT_BACKDROP) ||
       (j_state->struct_type == STRUCT_WINDOW))
    {
        json_new_state_struct(0, KEY_BORDER_TYPE);
        return;
    }

    json_error("Requires parent window or backdrop");
}

// -----------------------------------------------------------------------

static void key_vector(void)
{
    if(j_state->struct_type == STRUCT_MENU_ITEM)
    {
        json_new_state_struct(0, KEY_VECTOR);
        return;
    }

    json_error("Requires parent menu-item");
}

// -----------------------------------------------------------------------

static void key_shortcut(void)
{
    if(j_state->struct_type == STRUCT_MENU_ITEM)
    {
        json_new_state_struct(0, KEY_SHORTCUT);
        return;
    }

    json_error("Requires parent menu-item");
}

// -----------------------------------------------------------------------

static void key_flag(void)
{
    if(j_state->struct_type == STRUCT_FLAGS)
    {
        json_new_state_struct(0, KEY_FLAG);
        return;
    }

    json_error("Requires parent flags structure");
}

// -----------------------------------------------------------------------
// these hash values look like magic numbers but they are derived
// programatically by running this library as an executable.
// see ui_json.c

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
    { 0xaeb95d5b,  key_flag        }
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
// a key is a "quoted-name" used to name objects and keys

void json_state_key(void)
{
    int f;
    size_t len;

    // allows for syntatically incorrect placement of commas on the last
    // item within an object
    if (json_token[0] == '}')
    {
        // user put in a trailing comma, hence the unexpected right brace
        // so just handle this as a right brace...
        json_state_r_brace();
        return;
    }

    len = strlen(json_token);

    if((json_token[0]       != '"') ||
       (json_token[len - 1] != '"'))
    {
        json_error("Key names must be quoted");
        return;
    }

    strip_quotes(len);

    // objects are a type of key which are a container for keys
    f = re_switch(object_types, NUM_OBJECTS, json_hash);
    j_state->state = STATE_L_BRACE;

    // if reswitch returned -1 here then we did not just parse
    // in an object name but a possible key name

    if(f == -1)
    {
        f = re_switch(key_types, NUM_KEYS, json_hash);
        j_state->state = STATE_VALUE;

        if(f == -1)
        {
            json_error("Unknown key name");
        }
    }

    // every key / object must be followed by a colon

    token();
    json_hash = fnv_hash(json_token);

    if(json_hash != json_syntax[JSON_COLON])
    {
        json_error("Missing colon");
    }
}

// =======================================================================
