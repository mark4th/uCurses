// attribs.h
// -----------------------------------------------------------------------

#ifndef UC_ATTRIBS_H
#define UC_ATTRIBS_H

// -----------------------------------------------------------------------

#include "uCurses.h"

// -----------------------------------------------------------------------

typedef enum __attribute__((__packed__))
{
    BLACK,   RED,        GREEN,    BROWN,
    BLUE,    MAGENTA,    CYAN,     WHITE,
    GRAY,    LT_RED,     LT_GREEN, YELLOW,
    LT_BLUE, LT_MAGENTA, LT_CYAN,  LT_WHITE
} uC_color_t;

// -----------------------------------------------------------------------

typedef enum __attribute__((__packed__))
{
    DEFAULT_FG = WHITE,
    DEFAULT_BG = BLACK
} uC_default_clr_t;

// -----------------------------------------------------------------------

typedef enum __attribute__((__packed__))
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
// indicies into attributs array

typedef enum __attribute__((__packed__))
{
    ATTR,                   // uint_16_t
    FG = 2, FG_R = 2,       // uint8_t's
    BG = 3, BG_R = 3,
    FG_G,   BG_G,
    FG_B,   BG_B,
} uC_attr_index_t;

// -----------------------------------------------------------------------

typedef struct
{
    union
    {
        uint8_t bytes[8];
        uint64_t chunk;
    };
} uC_attribs_t;

// -----------------------------------------------------------------------
// default attribs, not used by json parser, there are no defaults there

#define uC_ATTRS_NORMAL   (0x0004030000000080)
#define uC_ATTRS_SELECTED (0x0001060000000080)
#define uC_ATTRS_DISABLED (0x00080400000000c2)

// -----------------------------------------------------------------------
// visibility hidden

void alloc_attr_grp(void);
void free_attr_grp(void);
void int_color_set(void);
void apply_attribs(void);

ti_attrib_t add_attr(uint8_t a, ti_attrib_t attr);

// -----------------------------------------------------------------------

API void uC_attr_set_attr(uC_attribs_t *attribs, ti_attrib_t attr);
API void uC_attr_clr_attr(uC_attribs_t *attribs, ti_attrib_t attr);
API void uC_attr_set_bytes(uC_attribs_t *attribs, uC_attr_index_t which,
    uC_color_t color);
API void uC_console_reset_attrs(void);
API void uC_console_set_fg(uC_color_t color);
API void uC_console_set_bg(uC_color_t color);
API void uC_console_set_gray_fg(uC_color_t color);
API void console_set_gray_bg(uC_color_t color);
API void uC_console_set_rgb_fg(uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_console_set_rgb_bg(uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_console_clr_attr(ti_attrib_t attr);
API void uC_console_set_bytes(uC_attr_index_t which, uC_color_t color);
API void uC_console_set_ul(void);
API void uC_console_set_rev(void);
API void uC_console_set_bold(void);
API void uC_console_clr_ul(void);
API void uC_console_clr_rev(void);
API void uC_console_clr_bold(void);

// -----------------------------------------------------------------------

#endif // UC_ATTRIBS_H

// =======================================================================
