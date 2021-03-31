// parsing json definitions for user interface layout
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------
// non rescursive parsing

typedef struct
{
    uint32_t state;         // ui sate
    void *structure;        // structure being populated
    uint32_t require;       // bit masks of pending required items
    char *data;
    size_t size;
    uint32_t index;         // parse index into data
    uint32_t line_end;      // index not updated till next line parsed
} ui_state_t;

// -----------------------------------------------------------------------

static ui_state_t *ui_state;

static list_t ui_stack;

char *linebuf;

// -----------------------------------------------------------------------
// list of templates found in the source json data

struct
{
    ui_state_t *attribs;
    ui_state_t *message;
    ui_state_t *backdrop;
    ui_state_t *window;
    ui_state_t *menu_item;
    ui_state_t *pulldown;
    ui_state_t *menu_bar;
} templates = { 0 };

// -----------------------------------------------------------------------

const char interp[] __attribute__((section(".interp"))) =
   "/lib64/ld-linux-x86-64.so.2";

// -----------------------------------------------------------------------

char banner[] = "\nlibuCurses v0.0.1\n";

// -----------------------------------------------------------------------
// i never do string comparisons on these

static char *keywords[] =
{
    "#",
    ":",
    "{",
    "}",
    "[",
    "]",
    "|attribs|",
    "|message|",
    "|backdrop|",
    "|window|",
    "|menu-item|",
    "|pulldown|",
    "|menu-bar|",
    "backdrop",
    "menu-item",
    "pulldown",
    "menu-barfg",
    "gray-fg",
    "rgb-fg",
    "bg",
    "gray-bg",
    "rgb-bg",
    "xco",
    "yco",
    "string",
    "attribs",
    "border-attribs",
    "border-type",
    "SINGLE",
    "DOUBLE",
    "ROUND",
    "NONE",
    "message",
    "window",
    "name",
    "width",
    "height",
    "relative-to",
    "position",
    "LEFT-OF",
    "RIGHT-OF",
    "ABOVE",
    "BELOWgravity",
    "TOP",
    "BOTTOM",
    "LEFT",
    "RIGHT",
    "padding",
    "flags",
    "LOCKED",
    "BOXED",
    "FILLED",
    "vector",
    "shortcut",
    "disabled",
    "menu-items",
    "pulldowns",
    "screen",
    "windows"
};

#define NUM_KEYS (sizeof(keywords) / sizeof(keywords[0]))

// -----------------------------------------------------------------------

void make_hash(void)
{
    uint16_t i;
    uint32_t key;
    char *s;

    for(i = 0; i < NUM_KEYS; i++)
    {
        s = keywords[i];
        key = fnv_hash(s);
        printf("%08x  ", key);
        printf("%s\n", s);
    }
}

// -----------------------------------------------------------------------

void entry(void)
{
    printf("%s", banner);

    printf("\nTop Secret: You No Look!\n");

    make_hash();

    printf("\n\n");
    _exit(0);
}

// -----------------------------------------------------------------------
// the above string table gives me these hash values which i sorted

static __attribute__((used)) uint32_t key_hash[] =
{
    0x050c5d3c,             // #
    0x050c5d25,             // :
    0x050c5d64,             // {
    0x050c5d62,             // }
    0x050c5d44,             // [
    0x050c5d42,             // ]
    0x885c3a16,             // |attribs|
    0x8776500c,             // |message|
    0x8bbde517,             // |backdrop|
    0x0aa7de35,             // |window|
    0x324d1b4e,             // |menu-item|
    0xcd2847fe,             // |pulldown|
    0x858b360a,             // |menu-bar|
    0x3bacc0d7,             // backdrop
    0x90f9ece0,             // menu-item
    0x09159434,             // pulldown
    0x39d8873d,             // menu-barfg
    0xaa3b6788,             // gray-fg
    0xea8606c2,             // rgb-fg
    0x6f772ba0,             // bg
    0xa63b61c4,             // gray-bg
    0xe686003e,             // rgb-bg
    0x1c63995d,             // xco
    0x3461800c,             // yco
    0xf202d8ce,             // string
    0xbc6bca20,             // attribs
    0x77d19b03,             // border-attribs
    0x362bb2fc,             // border-type
    0x6998100d,             // SINGLE
    0x79186806,             // DOUBLE
    0x522d832f,             // ROUND
    0xb15a4f9b,             // NONE
    0xdbfffa2a,             // message
    0x8ae7f465,             // window
    0x2f8b3bf4,             // name
    0x182e64eb,             // width
    0x4c47d5c0,             // height
    0x9de915b9,             // relative-to
    0x5e19d274,             // position
    0xf4010d8c,             // LEFT-OF
    0xd28fbfa1,             // RIGHT-OF
    0x3e16b6cc,             // ABOVE
    0xd1a2bfce,             // BELOWgravity
    0x201e7aaa,             // TOP
    0xcb19c820,             // BOTTOM
    0x4318cb32,             // LEFT
    0x25692569,             // RIGHT
    0x440b8034,             // padding
    0x68cdf632,             // flags
    0x726de363,             // LOCKED
    0xb62785fb,             // BOXED
    0xe5a76a7d,             // FILLED
    0x0ee694b4,             // vector
    0x1c13e01f,             // shortcut
    0xc1a03855,             // disabled
    0x196fe4d3,             // menu-items
    0x80f84daf,             // pulldowns
    0x2ff97421,             // screen
    0x1025ba8c              // windows
};

// -----------------------------------------------------------------------

void skip_comment(void)
{
    ui_state->index = ui_state->line_end;
}

// -----------------------------------------------------------------------
// |attribs|

void skip_template(ui_state_t **template)
{
    char c;
    char *s = ui_state->data;
    uint32_t i = ui_state->line_end;

    ui_state_t *q = calloc(1, sizeof(*q));
    q->data = &s[i];

    while(i != ui_state->size)
    {
       c = s[i];
       if(c == '}') { break; }
       i++; q->size++;
    }
    if(c != '}') { /* missing terminator */ }
    ui_state->line_end = i;
    ui_state->index = i;
    *template = q;
}

// -----------------------------------------------------------------------

void skip_attribs(void)    { skip_template(&templates.attribs); }
void skip_message(void)    { skip_template(&templates.message); }
void skip_backdrop(void)   { skip_template(&templates.backdrop); }
void skip_window(void)     { skip_template(&templates.window); }
void skip_menu_item(void)  { skip_template(&templates.menu_item); }
void skip_menu_pull(void ) { skip_template(&templates.pulldown); }
void skip_menu_bar(void )  { skip_template(&templates.menu_bar); }

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// copy next line of source data to line buffer

void load_line(void)
{
    char *s = ui_state->data;
    char *d = linebuf;
    uint32_t i = ui_state->line_end;
    ui_state->index = i;
    char c = 1;

    while((i != ui_state->size) && (c != 0))
    {
        c = s[i++];
        // so parser has less to interpret
        if(c == 0x09) { c = 0x20; }
        if(c == 0x0d) { c = 0; }
        *d++ = c;
    }
    ui_state->line_end = i;
}

// -----------------------------------------------------------------------

void reload_line(void)
{
    ui_state->line_end = ui_state->index;
    load_line();
}

// -----------------------------------------------------------------------

void json_parse_ui(char *json, size_t size)
{
    ui_state = calloc(1, sizeof(*ui_state));
    linebuf  = calloc(1024, sizeof(char));
    ui_state->data = json;
    ui_state->size = size;




    ui_stack.head = 0;
}

// -----------------------------------------------------------------------

// =======================================================================
