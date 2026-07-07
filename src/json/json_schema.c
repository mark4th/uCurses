// json_schema.c  - schema-driven JSON key/object dispatch
// -----------------------------------------------------------------------
// Replaces json_key.c.  The parent→child constraint rules that were
// encoded as ~20 individual C functions are now a single data table.
// One generic dispatcher does the work that all those functions did.
// -----------------------------------------------------------------------

#include <string.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_utils.h"
#include "json.h"

// -----------------------------------------------------------------------

#ifdef UC_JSON

// -----------------------------------------------------------------------

extern uC_screen_t   *active_screen;
extern json_state_t  *json_state;
extern json_vars_t   *json_vars;
extern const int32_t  json_syntax[3];

// -----------------------------------------------------------------------
// bitmask helper: bit N set means struct_type N is a valid parent

#define PM(x)  (1u << (x))

// all five attrib container types (attribs / b_attribs / s_attribs / ...)
#define PM_ATTRIB_TYPES  \
    (PM(STRUCT_ATTRIBS)   | PM(STRUCT_B_ATTRIBS) | \
     PM(STRUCT_S_ATTRIBS)  | PM(STRUCT_F_ATTRIBS) | \
     PM(STRUCT_D_ATTRIBS))

// objects that can directly contain an attribs block
#define PM_ATTRIB_HOSTS  \
    (PM(STRUCT_BACKDROP)  | PM(STRUCT_WINDOW) | \
     PM(STRUCT_PULLDOWN)   | PM(STRUCT_MENU_BAR))

// -----------------------------------------------------------------------
// schema entry flags

// would this be better an a packed enum or is this actually
// better?

#define SF_OBJ   0x01   // this is an object: expects a { } block
#define SF_ROOT  0x02   // must be outermost object (stack must be empty)
#define SF_SKIP  0x04   // silently consumed when parent == STRUCT_BACKDROP
#define SF_BREAK 0x08   // debug breakpoint: uC_noop(), no state created

// -----------------------------------------------------------------------

typedef struct
{
    int32_t  hash;           // FNV-1 hash of the JSON key string
    uint32_t valid_parents;  // parent bitmask (0 = root / no parent)
    uint16_t child_type;     // json_type_t assigned to the new state
    uint16_t child_size;     // sizeof the C struct to allocate (0 = none)
    uint8_t  flags;          // SF_* flags above
} json_schema_t;

// -----------------------------------------------------------------------
// Hash values are the same FNV-1 values already in the existing
// object_types[] and key_types[] dispatch tables in the old json_key.c.
// Objects are listed first so that when two entries share the same hash
// (the "flags" key), the object entry wins for ambiguous parent contexts.

static const json_schema_t schema[] =
{
    // ---- objects (SF_OBJ) -----------------------------------------------

    {
        0x2ff97421, 0,  STRUCT_SCREEN, sizeof(uC_screen_t),
        SF_OBJ | SF_ROOT
    },

    {
        0x1025ba8c, PM(STRUCT_SCREEN), STRUCT_WINDOWS,
        0, SF_OBJ
    },

    {
        0x8ae7f465, PM(STRUCT_WINDOWS),
          STRUCT_WINDOW,    sizeof(uC_window_t),   SF_OBJ
    },

    { 0x3bacc0d7, PM(STRUCT_SCREEN),
      STRUCT_BACKDROP,  sizeof(uC_window_t),   SF_OBJ           },

    { 0x95fe0788, PM(STRUCT_SCREEN),
      STRUCT_MENU_BAR,  sizeof(menu_bar_t),    SF_OBJ           },

    { 0x80f84daf, PM(STRUCT_MENU_BAR),
      STRUCT_PULLDOWNS, 0,                     SF_OBJ           },

    { 0x09159434, PM(STRUCT_PULLDOWNS),
      STRUCT_PULLDOWN,  sizeof(pulldown_t),    SF_OBJ           },

    { 0x196fe4d3, PM(STRUCT_PULLDOWN),
      STRUCT_MENU_ITEMS,0,                     SF_OBJ           },

    { 0x90f9ece0, PM(STRUCT_MENU_ITEMS),
      STRUCT_MENU_ITEM, sizeof(menu_item_t),   SF_OBJ           },

    { 0xbc6bca20, PM_ATTRIB_HOSTS,
      STRUCT_ATTRIBS,   sizeof(uC_attribs_t),  SF_OBJ           },

    { 0x77d19b03, PM(STRUCT_BACKDROP) | PM(STRUCT_WINDOW),
      STRUCT_B_ATTRIBS, sizeof(uC_attribs_t),  SF_OBJ           },

    { 0x4d8ce0ce, PM(STRUCT_PULLDOWN) | PM(STRUCT_MENU_BAR),
      STRUCT_S_ATTRIBS, sizeof(uC_attribs_t),  SF_OBJ           },

    { 0x92db923b, PM(STRUCT_WINDOW),
      STRUCT_F_ATTRIBS, sizeof(uC_attribs_t),  SF_OBJ           },

    { 0x19007641, PM(STRUCT_PULLDOWN) | PM(STRUCT_MENU_BAR),
      STRUCT_D_ATTRIBS, sizeof(uC_attribs_t),  SF_OBJ           },

    { 0xea8606c2, PM_ATTRIB_TYPES,
      STRUCT_RGB_FG,    0,                     SF_OBJ           },

    { 0xe686003e, PM_ATTRIB_TYPES,
      STRUCT_RGB_BG,    0,                     SF_OBJ           },

    // "flags" as an object container (comes before "flags" as a key so
    // the object interpretation wins for any shared parent contexts)
    { 0x68cdf632, PM_ATTRIB_HOSTS,
      STRUCT_FLAGS,     0,                     SF_OBJ           },

    // ---- scalar keys (no SF_OBJ) ----------------------------------------

    { 0x6b77251c, PM_ATTRIB_TYPES, KEY_FG,          0, 0 },
    { 0x6f772ba0, PM_ATTRIB_TYPES, KEY_BG,          0, 0 },
    { 0xaa3b6788, PM_ATTRIB_TYPES, KEY_GRAY_FG,     0, 0 },
    { 0xa63b61c4, PM_ATTRIB_TYPES, KEY_GRAY_BG,     0, 0 },

    { 0x3a72d292, PM(STRUCT_RGB_FG) | PM(STRUCT_RGB_BG), KEY_RED,   0, 0 },
    { 0xf73297b2, PM(STRUCT_RGB_FG) | PM(STRUCT_RGB_BG), KEY_GREEN, 0, 0 },
    { 0x4f068569, PM(STRUCT_RGB_FG) | PM(STRUCT_RGB_BG), KEY_BLUE,  0, 0 },

    { 0x1c63995d, PM(STRUCT_WINDOW), KEY_XCO,         0, 0 },
    { 0x3461800c, PM(STRUCT_WINDOW), KEY_YCO,         0, 0 },
    { 0x182e64eb, PM(STRUCT_WINDOW), KEY_WIDTH,        0, 0 },
    { 0x4c47d5c0, PM(STRUCT_WINDOW), KEY_HEIGHT,       0, 0 },

    { 0x2f8b3bf4,
      PM(STRUCT_PULLDOWN) | PM(STRUCT_MENU_ITEM) | PM(STRUCT_WINDOW),
      KEY_NAME,  0, 0 },

    // "flags" as a scalar value; only reaches here for parents not covered
    // by the object entry above (i.e. STRUCT_MENU_ITEM)
    { 0x68cdf632,
      PM(STRUCT_PULLDOWN) | PM(STRUCT_MENU_ITEM) | PM(STRUCT_WINDOW),
      KEY_FLAGS, 0, 0 },

    { 0x362bb2fc, PM(STRUCT_BACKDROP) | PM(STRUCT_WINDOW), KEY_BORDER_TYPE, 0, 0 },
    { 0x0ee694b4, PM(STRUCT_MENU_ITEM), KEY_VECTOR,   0, 0 },
    { 0x1c13e01f, PM(STRUCT_MENU_ITEM), KEY_SHORTCUT, 0, 0 },
    { 0xaeb95d5b, PM(STRUCT_FLAGS),     KEY_FLAG,      0, 0 },
    { 0xe960add1, PM(STRUCT_BACKDROP) | PM(STRUCT_WINDOW), KEY_BLANK, 0, 0 },

    // silently ignored when parent is backdrop; consumed but no action taken
    { 0x4fc0385f, PM(STRUCT_BACKDROP) | PM(STRUCT_WINDOW),
      KEY_ORDER, 0, SF_SKIP },

    // debug hook: put a breakpoint on uC_noop() and add to any JSON object
    { 0x1441d80c, 0, 0, 0, SF_BREAK },
};

#define NUM_SCHEMA  (sizeof(schema) / sizeof(schema[0]))

// -----------------------------------------------------------------------
// find the first schema entry matching hash where the current parent
// is allowed.  pmask is a bitmask of the current parent struct_type
// (0 at root, 1<<struct_type otherwise).  valid_parents == 0 in the
// table means root-only: matches only when pmask == 0.
// SF_BREAK entries always match regardless of parent.

static const json_schema_t *find_schema(int32_t hash, uint32_t pmask)
{
    uint16_t i;

    for (i = 0; i < NUM_SCHEMA; i++)
    {
        if (schema[i].hash != hash)
        {
            continue;
        }

        if (schema[i].flags & SF_BREAK)
        {
            return &schema[i];
        }

        if (!schema[i].valid_parents)
        {
            if (!pmask)
            {
                return &schema[i];   // root-only entry, root context
            }
            continue;
        }

        if (schema[i].valid_parents & pmask)
        {
            return &schema[i];
        }
    }

    return NULL;
}

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

static void json_schema_dispatch(void)
{
    // json_type_t is a packed (uint8_t) enum; the root state is initialised
    // with struct_type = -1 which wraps to 255.  Use the stack count to
    // detect root instead of a signed comparison against -1, then build
    // a bitmask that is 0 at root or (1 << struct_type) otherwise.

    uint32_t pmask = json_vars->json_stack.count
        ? (1u << (uint32_t)json_state->struct_type)
        : 0;

    const json_schema_t *s = find_schema(json_vars->json_hash, pmask);

    if (!s)
    {
        json_error("Unknown key or invalid context");
    }

    // --- debug breakpoint: uC_noop, no state, consume as scalar value ---

    if (s->flags & SF_BREAK)
    {
        uC_noop();
        json_state->state = JSON_STATE_VALUE;
        return;
    }

    // --- screen: must be outermost, one-per-file, needs extra init -----

    if (s->flags & SF_ROOT)
    {
        if (json_vars->json_stack.count != 0)
        {
            json_error("Screen must be the first object defined!");
        }

        if (active_screen != NULL)
        {
            json_error("There can be only one screen! (for now?)");
        }

        json_new_state_struct(s->child_size, s->child_type);

        uC_screen_t *scr  = json_state->structure;
        scr->width        = json_vars->console_width;
        scr->height       = json_vars->console_height;
        active_screen     = scr;

        json_state->state = JSON_STATE_L_BRACE;
        return;
    }

    // --- silently consumed key (e.g. "order" inside a backdrop) --------
    // push a throwaway state so json_state_value's json_pop() is balanced

    if ((s->flags & SF_SKIP) && (json_state->struct_type == STRUCT_BACKDROP))
    {
        json_new_state_struct(0, 0);
        json_state->state = JSON_STATE_VALUE;
        return;
    }

    // --- common case: create child state, set next expected token --------

    json_new_state_struct(s->child_size, s->child_type);

    json_state->state = (s->flags & SF_OBJ)
        ? JSON_STATE_L_BRACE
        : JSON_STATE_VALUE;
}

// -----------------------------------------------------------------------

void json_state_key(void)
{
    size_t len;

    // trailing comma leaves a stray } as the next token
    if (json_vars->json_token[0] == '}')
    {
        json_state_r_brace();
        return;
    }

    len = strlen((char *)json_vars->json_token);
    must_quote(len);
    strip_quotes(len);

    json_schema_dispatch();

    check_colon();
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
