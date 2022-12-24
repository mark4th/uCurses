// attribs.h
// -----------------------------------------------------------------------

#ifndef ATTRIBS_H
#define ATTRIBS_H

// -----------------------------------------------------------------------

#include "uCurses.h"

// -----------------------------------------------------------------------

typedef enum
{
    BLACK,   RED,        GREEN,    BROWN,
    BLUE,    MAGENTA,    CYAN,     WHITE,
    GRAY,    LT_RED,     LT_GREEN, YELLOW,
    LT_BLUE, LT_MAGENTA, LT_CYAN,  LT_WHITE
} color_t;

// -----------------------------------------------------------------------

enum
{
    DEFAULT_FG = WHITE,
    DEFAULT_BG = BLACK
};

// -----------------------------------------------------------------------

typedef enum
{
    TI_UNDERLINE,
    TI_REVERSE,
    TI_BOLD,

    TI_FG_RGB,
    TI_BG_RGB,
    TI_FG_GRAY,
    TI_BG_GRAY,

    UNDERLINE = (1 << TI_UNDERLINE),
    REVERSE   = (1 << TI_REVERSE),
    BOLD      = (1 << TI_BOLD),

    FG_RGB    = (1 << TI_FG_RGB),
    BG_RGB    = (1 << TI_BG_RGB),
    FG_GRAY   = (1 << TI_FG_GRAY),
    BG_GRAY   = (1 << TI_BG_GRAY),
} ti_attrib_t;

// -----------------------------------------------------------------------
// indicies into attributes array

typedef enum
{
    ATTR,                   // uint_16_t
    FG = 2, FG_R = 2,       // uint8_t's
    BG = 3, BG_R = 3,
    FG_G,   BG_G,
    FG_B,   BG_B,
} attr_index_t;

// -----------------------------------------------------------------------

typedef struct
{
    union
    {
        uint8_t bytes[8];
        uint64_t chunk;
    };
} attribs_t;

// -----------------------------------------------------------------------

typedef struct
{
    attribs_t attrs;
    attribs_t old_attrs;
} attr_grp_t;

// -----------------------------------------------------------------------
// usually you would use the functions and macros below to mess with this

extern attr_grp_t *attr_grp;

// -----------------------------------------------------------------------
// visibility hidden

void alloc_attr_grp(void);
void free_attr_grp(void);
void int_color_set(void);
void apply_attribs(void);

ti_attrib_t add_attr(uint8_t a, ti_attrib_t attr);

// -----------------------------------------------------------------------

API void uC_attr_set_attr(attribs_t *attribs, ti_attrib_t attr);
API void uC_attr_clr_attr(attribs_t *attribs, ti_attrib_t attr);
API void uC_attr_set_bytes(attribs_t *attribs, attr_index_t which,
    color_t color);
API void uC_console_reset_attrs(void);
API void uC_console_set_fg(color_t color);
API void uC_console_set_bg(color_t color);
API void uC_console_set_gray_fg(color_t color);
API void console_set_gray_bg(color_t color);
API void uC_console_set_rgb_fg(color_t r, color_t g, color_t b);
API void uC_console_set_rgb_bg(color_t r, color_t g, color_t b);
API void uC_console_clr_attr(ti_attrib_t attr);
API void uC_console_set_bytes(attr_index_t which, color_t color);
API void uC_console_set_ul(void);
API void uC_console_set_rev(void);
API void uC_console_set_bold(void);
API void uC_console_clr_ul(void);
API void uC_console_clr_rev(void);
API void uC_console_clr_bold(void);

// -----------------------------------------------------------------------

#endif // ATTRIBS_H

// =======================================================================
