// json_key.c   json key name parsing
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "uCurses.h"
#include "uC_json.h"
#include "uC_utils.h"
#include "uC_window.h"
#include "uC_switch.h"

// -----------------------------------------------------------------------

extern uC_screen_t *active_screen;
extern json_state_t *json_state;
extern json_vars_t *json_vars;
extern const int32_t json_syntax[3];

// -----------------------------------------------------------------------
// must be the first object specified

static void struct_screen(void)
{
    uC_screen_t *scr;

    if (json_vars->json_stack.count == 0)
    {
        json_new_state_struct(sizeof(uC_screen_t), STRUCT_SCREEN);
        scr = json_state->structure;

        // you have no say in this
        scr->width  = json_vars->console_width;
        scr->height = json_vars->console_height;

        active_screen = scr;
        return;
    }

    json_error("There can be only one!");
}

// -----------------------------------------------------------------------

static void struct_windows(void)
{
    if (json_state->struct_type == STRUCT_SCREEN)
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
    if (json_state->struct_type == STRUCT_WINDOWS)
    {
        json_new_state_struct(sizeof(uC_window_t), STRUCT_WINDOW);
        return;
    }

    json_error("Requires parent windows structure");
}

// -----------------------------------------------------------------------
// special non writeable window (well you can but usually dont :)

static void struct_backdrop(void)
{
    if (json_state->struct_type == STRUCT_SCREEN)
    {
        json_new_state_struct(sizeof(uC_window_t), STRUCT_BACKDROP);
        return;
    }

    json_error("Requires parent screen");
}

// -----------------------------------------------------------------------

static void struct_m_bar(void)
{
    if (json_state->struct_type == STRUCT_SCREEN)
    {
        json_new_state_struct(sizeof(menu_bar_t), STRUCT_MENU_BAR);
        return;
    }

    json_error("Requires parent screen");
}

// -----------------------------------------------------------------------

static void struct_pulldowns(void)
{
    if (json_state->struct_type == STRUCT_MENU_BAR)
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
    if (json_state->struct_type == STRUCT_PULLDOWNS)
    {
        json_new_state_struct(sizeof(pulldown_t), STRUCT_PULLDOWN);
        return;
    }

    json_error("Requires parent pulldowns structure");
}

// -----------------------------------------------------------------------

static void struct_m_items(void)
{
    if (json_state->struct_type == STRUCT_PULLDOWN)
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
    if (json_state->struct_type == STRUCT_MENU_ITEMS)
    {
        json_new_state_struct(sizeof(menu_item_t), STRUCT_MENU_ITEM);
        return;
    }

    json_error("Requires parent menu-items");
}

// -----------------------------------------------------------------------
// general attributes

static void struct_attribs(void)
{
    if ((json_state->struct_type == STRUCT_BACKDROP) ||
        (json_state->struct_type == STRUCT_WINDOW)   ||
        (json_state->struct_type == STRUCT_PULLDOWN) ||
        (json_state->struct_type == STRUCT_MENU_BAR))
    {
        json_new_state_struct(sizeof(uC_attribs_t), STRUCT_ATTRIBS);
        return;
    }

    json_error("Requires parent backdrop, window, pulldown or menu-bar");
}

// -----------------------------------------------------------------------
// border attributes

static void struct_b_attribs(void)
{
    if ((json_state->struct_type == STRUCT_BACKDROP) ||
        (json_state->struct_type == STRUCT_WINDOW))
    {
        json_new_state_struct(sizeof(uC_attribs_t), STRUCT_B_ATTRIBS);
        return;
    }

    json_error("Requires parent backdrop or window");
}

// -----------------------------------------------------------------------
// selected attributes

static void struct_s_attribs(void)
{
    if ((json_state->struct_type == STRUCT_PULLDOWN) ||
        (json_state->struct_type == STRUCT_MENU_BAR))
    {
        json_new_state_struct(sizeof(uC_attribs_t), STRUCT_S_ATTRIBS);
        return;
    }

    json_error("Requires parent pulldown or menu-bar");
}

// -----------------------------------------------------------------------
// focussed attributes

static void struct_f_attribs(void)
{
    if (json_state->struct_type == STRUCT_WINDOW)
    {
        json_new_state_struct(sizeof(uC_attribs_t), STRUCT_F_ATTRIBS);
        return;
    }

    json_error("Requires parent pulldown or menu-bar");
}

// -----------------------------------------------------------------------
// disabled attributes

static void struct_d_attribs(void)
{
    if ((json_state->struct_type == STRUCT_PULLDOWN) ||
        (json_state->struct_type == STRUCT_MENU_BAR))
    {
        json_new_state_struct(sizeof(uC_attribs_t), STRUCT_D_ATTRIBS);
        return;
    }

    json_error("Requires parent pulldown or menu bar");
}

// -----------------------------------------------------------------------

static void struct_rgb_fg(void)
{
    if ((json_state->struct_type == STRUCT_ATTRIBS)   ||
        (json_state->struct_type == STRUCT_B_ATTRIBS) ||
        (json_state->struct_type == STRUCT_S_ATTRIBS) ||
        (json_state->struct_type == STRUCT_F_ATTRIBS) ||
        (json_state->struct_type == STRUCT_D_ATTRIBS))
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
    if ((json_state->struct_type == STRUCT_ATTRIBS)   ||
        (json_state->struct_type == STRUCT_B_ATTRIBS) ||
        (json_state->struct_type == STRUCT_S_ATTRIBS) ||
        (json_state->struct_type == STRUCT_F_ATTRIBS) ||
        (json_state->struct_type == STRUCT_D_ATTRIBS))
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
    if ((json_state->struct_type == STRUCT_BACKDROP)  ||
        (json_state->struct_type == STRUCT_WINDOW)    ||
        (json_state->struct_type == STRUCT_PULLDOWN)  ||
        (json_state->struct_type == STRUCT_MENU_BAR))
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
    if ((json_state->struct_type == STRUCT_ATTRIBS)   ||
        (json_state->struct_type == STRUCT_B_ATTRIBS) ||
        (json_state->struct_type == STRUCT_S_ATTRIBS) ||
        (json_state->struct_type == STRUCT_F_ATTRIBS) ||
        (json_state->struct_type == STRUCT_D_ATTRIBS))
    {
        json_new_state_struct(0, key);
        return;
    }

    json_error("Requires parent atrribs structure");
}

// -----------------------------------------------------------------------

static void key_fg(void)      { key_attr(KEY_FG);      }
static void key_bg(void)      { key_attr(KEY_BG);      }
static void key_gray_fg(void) { key_attr(KEY_GRAY_FG); }
static void key_gray_bg(void) { key_attr(KEY_GRAY_BG); }

// -----------------------------------------------------------------------

static void key_rgb(uint16_t key)
{
    if ((json_state->struct_type == STRUCT_RGB_FG) ||
        (json_state->struct_type == STRUCT_RGB_BG))
    {
        json_new_state_struct(0, key);
        return;
    }

    json_error("Requires parent RGB");
}

// -----------------------------------------------------------------------

static void key_red(void)   { key_rgb(KEY_RED);   }
static void key_green(void) { key_rgb(KEY_GREEN); }
static void key_blue(void)  { key_rgb(KEY_BLUE);  }

// -----------------------------------------------------------------------

static void key_xywh(uint16_t key)
{
    if (json_state->struct_type == STRUCT_WINDOW)
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
    if ((json_state->struct_type == STRUCT_PULLDOWN)  ||
        (json_state->struct_type == STRUCT_MENU_ITEM) ||
        (json_state->struct_type == STRUCT_WINDOW))
    {
        json_new_state_struct(0, KEY_NAME);
        return;
    }

    json_error("Requires parent pulldown or menu-item");
}

// -----------------------------------------------------------------------

static void key_flags(void)
{
    if ((json_state->struct_type == STRUCT_PULLDOWN)  ||
        (json_state->struct_type == STRUCT_MENU_ITEM) ||
        (json_state->struct_type == STRUCT_WINDOW))
    {
        json_new_state_struct(0, KEY_FLAGS);
        return;
    }

    json_error("Requires parent window, pulldown or menu-item");
}

// -----------------------------------------------------------------------

static void key_border_type(void)
{
    if ((json_state->struct_type == STRUCT_BACKDROP) ||
        (json_state->struct_type == STRUCT_WINDOW))
    {
        json_new_state_struct(0, KEY_BORDER_TYPE);
        return;
    }

    json_error("Requires parent window or backdrop");
}

// -----------------------------------------------------------------------

static void key_vector(void)
{
    if (json_state->struct_type == STRUCT_MENU_ITEM)
    {
        json_new_state_struct(0, KEY_VECTOR);
        return;
    }

    json_error("Requires parent menu-item");
}

// -----------------------------------------------------------------------

static void key_shortcut(void)
{
    if (json_state->struct_type == STRUCT_MENU_ITEM)
    {
        json_new_state_struct(0, KEY_SHORTCUT);
        return;
    }

    json_error("Requires parent menu-item");
}

// -----------------------------------------------------------------------

static void key_flag(void)
{
    if (json_state->struct_type == STRUCT_FLAGS)
    {
        json_new_state_struct(0, KEY_FLAG);
        return;
    }

    json_error("Requires parent flags structure");
}

// -----------------------------------------------------------------------
// put a breakpoint on here, place a breakpoint key in your json

// breakpoint : xxx   // any value

static void breakpoint(void)
{
    uC_noop();
}

// -----------------------------------------------------------------------
// these hash values look like magic numbers but they are derived
// programatically by running this library as an executable.
// see ui_json.c

static const uC_switch_t key_types[] =
{
    { 0x6b77251c, key_fg     }, { 0xaa3b6788, key_gray_fg     },
    { 0x6f772ba0, key_bg     }, { 0xa63b61c4, key_gray_bg     },
    { 0x3a72d292, key_red    }, { 0xf73297b2, key_green       },
    { 0x4f068569, key_blue   }, { 0x1c63995d, key_xco         },
    { 0x3461800c, key_yco    }, { 0x182e64eb, key_width       },
    { 0x4c47d5c0, key_height }, { 0x2f8b3bf4, key_name        },
    { 0x68cdf632, key_flags  }, { 0x362bb2fc, key_border_type },
    { 0x0ee694b4, key_vector }, { 0x1c13e01f, key_shortcut    },
    { 0xaeb95d5b, key_flag   }, { 0x1441d80c, breakpoint      },
};

#define NUM_KEYS (sizeof(key_types) / sizeof(key_types[0]))

// -----------------------------------------------------------------------

static const uC_switch_t object_types[] =
{
    { 0x2ff97421, struct_screen    }, { 0x1025ba8c, struct_windows   },
    { 0x8ae7f465, struct_window    }, { 0x3bacc0d7, struct_backdrop  },
    { 0x95fe0788, struct_m_bar     }, { 0x80f84daf, struct_pulldowns },
    { 0x09159434, struct_pulldown  }, { 0x196fe4d3, struct_m_items   },
    { 0x90f9ece0, struct_m_item    }, { 0xbc6bca20, struct_attribs   },
    { 0x77d19b03, struct_b_attribs }, { 0x4d8ce0ce, struct_s_attribs },
    { 0x19007641, struct_d_attribs }, { 0x92db923b, struct_f_attribs },
    { 0xea8606c2, struct_rgb_fg    }, { 0xe686003e, struct_rgb_bg    },
    { 0x68cdf632, struct_flags     }
};

#define NUM_OBJECTS (sizeof(object_types) / sizeof(object_types[0]))

// -----------------------------------------------------------------------

static void must_quote(int16_t len)
{
    if ((json_vars->json_token[0]       != '"') ||
        (json_vars->json_token[len - 1] != '"'))
    {
        json_error("Key names must be quoted");
    }
}

// -----------------------------------------------------------------------
// if token was not an object it must be a key... is it?

static void is_key(void)
{
    int f;

    f = uC_switch(key_types, NUM_KEYS, json_vars->json_hash);
    json_state->state = STATE_VALUE;

    if (f == -1)
    {
        json_error("Unknown key name");
    }
}

// -----------------------------------------------------------------------
// every key / object must be followed by a colon

static void check_colon(void)
{
    token();
    json_vars->json_hash = fnv_hash(json_vars->json_token);

    if (json_vars->json_hash != json_syntax[JSON_COLON])
    {
        json_error("Missing colon");
    }
}

// -----------------------------------------------------------------------
// a key is a "quoted-name" used to name objects and keys

void json_state_key(void)
{
    int f;
    size_t len;

    // allows for syntatically incorrect placement of commas on the last
    // item within an object Thanks to Dwight Schaueer for this "fix"

    if (json_vars->json_token[0] == '}')
    {
        // user put in a trailing comma, hence the unexpected right brace
        // so just handle this as a right brace...
        json_state_r_brace();
        return;
    }

    len = strlen(json_vars->json_token);
    must_quote(len);
    strip_quotes(len);

    // objects are a type of key which are a container for keys
    f = uC_switch(object_types, NUM_OBJECTS, json_vars->json_hash);
    json_state->state = STATE_L_BRACE;

    // if uC_switch returned -1 here then we did not just parse
    // in an object name but a possible key name

    if (f == -1) { is_key(); }

    check_colon();
}

// =======================================================================
