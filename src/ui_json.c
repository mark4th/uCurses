// ui_json.c
// -----------------------------------------------------------------------

// technicaly not really part of the UI build but included for now so I
// can generate the hash values for all the various json keys.
// this should be a totally separate make target really

// -----------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------
// i never do string comparisons on these

static char *json_syntax[] =
{
    "#", ":", "{", "}", "[", "]",
};

// -----------------------------------------------------------------------

static char *ui_structs[] =
{
    "screen",
    "windows",
    "window",
    "backdrop",
    "menu-bar",
    "pulldowns",
    "pulldown",
    "menu-items",
    "menu-item",
    "attribs",
    "border-attribs",
    "selected-attribs",
    "disabled-attribs",
    "rgb-fg",
    "rgb-bg",
    "flags"
};

// -----------------------------------------------------------------------

static char *ui_values[] =
{
    "fg",    "gray-fg",
    "bg",    "gray-bg",
    "red",   "green",   "blue",
    "xco",   "yco",
    "width", "height",
    "name",  "flags",
    "border-type",
    "vector",
    "shortcut",
    "flag"
};

// -----------------------------------------------------------------------

static char *ui_constants[] =
{
    "MENU_DISABLED",
    "BDR_SINGLE",
    "BDR_DOUBLE",
    "BDR_CURVED",
    "WIN_LOCKED",
    "WIN_FILLED",
    "WIN_BOXED",
    "FAR",
    "BLACK",   "RED",        "GREEN",    "BROWN",
    "BLUE",    "MAGENTA",    "CYAN",     "WHITE",
    "GRAY",    "PINK",       "LT_GREEN", "YELLOW",
    "LT_BLUE", "LT_MAGENTA", "CYAN",     "LT_WHITE"
};

// -----------------------------------------------------------------------

#define J_SYNTAX        (sizeof(json_syntax)  / sizeof(json_syntax[0]))
#define UI_STRUCTS      (sizeof(ui_structs)   / sizeof(ui_structs[0]))
#define UI_VALUES       (sizeof(ui_values)    / sizeof(ui_values[0]))
#define UI_CONSTANTS    (sizeof(ui_constants) / sizeof(ui_constants[0]))

// -----------------------------------------------------------------------

void make_hash(void)
{
    uint16_t i, j;
    uint32_t key;
    char *s;
    char **p;

    char **keywords[] =
    {
        json_syntax, ui_structs, ui_values, ui_constants
    };

    uint16_t sizes[] =
    {
        J_SYNTAX, UI_STRUCTS, UI_VALUES, UI_CONSTANTS
    };

    #define NUMK (sizeof(keywords) / sizeof(keywords[0]))

    for(i = 0; i < NUMK; i++)
    {
        p = keywords[i];

        for(j = 0; j < sizes[i]; j++)
        {
             s = p[j];
             key = fnv_hash(s);
             printf("0x%08x  ", key);
             printf("%s\n", s);
        }
        printf("\n");
    }
}

// -----------------------------------------------------------------------

const char interp[] __attribute__((section(".interp"))) =
   "/lib64/ld-linux-x86-64.so.2";

char banner[] = "\nlibuCurses v0.0.1\n";

// -----------------------------------------------------------------------

void entry(void)
{
    printf("%s", banner);

    printf("\nTop Secret: You No Look!\n\n");

    make_hash();

    printf("\n\n");
    _exit(0);
}

// =======================================================================
