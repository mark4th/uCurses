// color.h
// -----------------------------------------------------------------------

    #pragma once

// -----------------------------------------------------------------------

typedef enum
{
    BLACK, RED, GREEN, BROWN,
    BLUE, MAGENTA, CYAN, WHITE,
    GRAY, LT_RED, LT_GREEN, YELLOW,
    LT_BLUE, LT_MAGENTA, LT_CYAN, LT_WHITE
} ti_color_t;

// -----------------------------------------------------------------------

typedef enum
{
    TI_UNDERLINE,  TI_REVERSE,   TI_BOLD,     TI_BLINK,
    TI_FG_RGB,     TI_BG_RGB,    TI_FG_GRAY,  TI_BG_GRAY,

    UNDERLINE = (1 << TI_UNDERLINE),
    REVERSE   = (1 << TI_REVERSE),
    BOLD      = (1 << TI_BOLD),
    BLINK     = (1 << TI_BLINK),

    FG_RGB    = (1 << TI_FG_RGB),
    BG_RGB    = (1 << TI_BG_RGB),
    FG_GRAY   = (1 << TI_FG_GRAY),
    BG_GRAY   = (1 << TI_BG_GRAY),
} ti_attrib_t;

// -----------------------------------------------------------------------
// indicies into attributes array

enum
{
    ATTR,                   // attribute flags
    FG_R,                   // rgb fg red
    FG_G,                   // rgb fg green
    FG_B,                   // rgb fg blue
    BG_R,                   // rgb bg red
    FG,    BG_G = 5,        // foreground or rgb bg green
    BG,    BG_B = 6,        // background or rgb bg blue
    UNUSED
};

// -----------------------------------------------------------------------

void set_ul(void);
void set_rev(void);
void set_bold(void);
void set_blink(void);

void clr_ul(void);
void clr_rev(void);
void clr_bold(void);
void clr_blink(void);

void set_gray_fg(uint8_t c);
void set_gray_bg(uint8_t c);

void set_rgb_fg(uint8_t r, uint8_t g, uint8_t b);
void set_rgb_bg(uint8_t r, uint8_t g, uint8_t b);

void set_fg(uint8_t c);
void set_bg(uint8_t c);

void set_norm(void);

// =======================================================================
