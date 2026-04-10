// uC_attribs.h
// -----------------------------------------------------------------------

#ifndef UC_ATTRIBS_H
#define UC_ATTRIBS_H

// -----------------------------------------------------------------------

#include "uCurses.h"

// -----------------------------------------------------------------------

typedef enum
{
   uC_COLOR_BLACK,    uC_COLOR_RED,
   uC_COLOR_GREEN,    uC_COLOR_BROWN,
   uC_COLOR_BLUE,     uC_COLOR_MAGENTA,
   uC_COLOR_CYAN,     uC_COLOR_WHITE,
   uC_COLOR_GRAY,     uC_COLOR_LT_RED,
   uC_COLOR_LT_GREEN, uC_COLOR_YELLOW,
   uC_COLOR_LT_BLUE,  uC_COLOR_LT_MAGENTA,
   uC_COLOR_LT_CYAN,  uC_COLOR_LT_WHITE
} __attribute__((__packed__))  uC_color_t;

// -----------------------------------------------------------------------

typedef enum
{
    uC_GRAY_00, uC_GRAY_01, uC_GRAY_02, uC_GRAY_03,
    uC_GRAY_04, uC_GRAY_05, uC_GRAY_06, uC_GRAY_07,
    uC_GRAY_08, uC_GRAY_09, uC_GRAY_10, uC_GRAY_11,
    uC_GRAY_12, uC_GRAY_13, uC_GRAY_14, uC_GRAY_15,
    uC_GRAY_16, uC_GRAY_17, uC_GRAY_18, uC_GRAY_19,
    uC_GRAY_20, uC_GRAY_21, uC_GRAY_22, uC_GRAY_23
} __attribute__((__packed__)) uC_colors_gray_t;

// -----------------------------------------------------------------------

typedef enum
{
    uC_DEFAULT_FG = uC_COLOR_WHITE,
    uC_DEFAULT_BG = uC_COLOR_BLACK
} __attribute__((__packed__))  uC_default_clr_t;

// -----------------------------------------------------------------------

enum
{
    UL,     REV,     BOLD,    RGB_FG,
    RGB_BG, GRAY_FG, GRAY_BG,

    ATTR_FLAG_UL      = (1 << UL),
    ATTR_FLAG_REV     = (1 << REV),
    ATTR_FLAG_BOLD    = (1 << BOLD),
    ATTR_FLAG_RGB_FG  = (1 << RGB_FG),
    ATTR_FLAG_RGB_BG  = (1 << RGB_BG),
    ATTR_FLAG_GRAY_FG = (1 << GRAY_FG),
    ATTR_FLAG_GRAY_BG = (1 << GRAY_BG),
} __attribute__((__packed__)) ;

// -----------------------------------------------------------------------
// atrtibute flags

typedef struct
{
    union
    {
        struct
        {
            uint8_t ul      : 1;  // character is underlined
            uint8_t rev     : 1;  // character is reverse video
            uint8_t bold    : 1;  // character is bold
            uint8_t rgb_fg  : 1;  // characters foreground is rgb
            uint8_t rgb_bg  : 1;  // characters background is rgb
            uint8_t gray_fg : 1;  // characters foreground is gray scale
            uint8_t gray_bg : 1;  // characters background is gray scale
            uint8_t r1      : 1;  // reserved bits
        } __attribute__((__packed__));
        uint16_t bits;
    } __attribute__((__packed__));
} __attribute__((__packed__)) uC_ti_attr_flags_t;

// -----------------------------------------------------------------------

typedef struct
{
    union
    {
        struct
        {
            uC_ti_attr_flags_t flags;

            union
            {
                uint8_t fg;
                uint8_t fg_r;
                uint8_t fg_gray;
            } __attribute__((__packed__));
            union
            {
                uint8_t bg;
                uint8_t bg_r;
                uint8_t bg_gray;
            } __attribute__((__packed__));
            uint8_t fg_g;           // foreground green
            uint8_t bg_g;           // background green
            uint8_t fg_b;           // foreground blue
            uint8_t bg_b;
            uint16_t reserved;      // im sure this will have a use!
        } __attribute__((__packed__));
        uint64_t blob;
    };
} __attribute__((__packed__))uC_attribs_t;

// -----------------------------------------------------------------------
// visibility hidden

void alloc_attr_grp(void);
void free_attr_grp(void);
void int_color_set(void);
void apply_attribs(void);

// -----------------------------------------------------------------------

API void uC_set_fg(uC_attribs_t *attr, uC_color_t color);
API void uC_set_bg(uC_attribs_t *attr, uC_color_t color);

API void uC_set_gray_fg(uC_attribs_t *attr, uC_colors_gray_t color);
API void uC_set_gray_bg(uC_attribs_t *attr, uC_colors_gray_t color);
API void uC_set_rgb_fg(uC_attribs_t *attr, uC_color_t r, uC_color_t g,
           uC_color_t b);
API void uC_set_rgb_bg(uC_attribs_t *attr, uC_color_t r, uC_color_t g,
           uC_color_t b);

API void uC_attr_set_flags(uC_attribs_t *attribs, uint16_t bits);
API void uC_attr_clr_flags(uC_attribs_t *attribs, uint16_t bits);

API void uC_console_clr_attr(int16_t flags);
API void uC_console_reset_attrs(void);
API void uC_console_set_fg(uC_color_t color);
API void uC_console_set_bg(uC_color_t color);
API void uC_console_set_gray_fg(uC_colors_gray_t color);
API void uC_console_set_gray_bg(uC_colors_gray_t color);
API void uC_console_set_rgb_fg(uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_console_set_rgb_bg(uC_color_t r, uC_color_t g, uC_color_t b);

// -----------------------------------------------------------------------

#endif // UC_ATTRIBS_H

// =======================================================================
