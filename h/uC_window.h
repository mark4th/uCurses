// window.h
// -----------------------------------------------------------------------

#ifndef WINDOW_H
#define WINDOW_H

#include "uCurses.h"
#include "uC_attribs.h"

// -----------------------------------------------------------------------
// structure of each cell of a window / screen

typedef struct
{
    attribs_t attrs;        // bold, underline, gray scale, rgb
    int32_t code;           // utf-8 codepoint
} cell_t;

// -----------------------------------------------------------------------

enum
{
    BDR_SINGLE,
    BDR_DOUBLE,
    BDR_CURVED
};

// -----------------------------------------------------------------------

enum
{
    SOLID = 0x2592,         // utf-8 'blank' char for backdrop
};

// -----------------------------------------------------------------------
// border.h ???

typedef enum
{
    BDR_TOP_LEFT,    BDR_TOP_RIGHT,
    BDR_BOTTOM_LEFT, BDR_BOTTOM_RIGHT,
    BDR_HORIZONTAL,  BDR_VERTICAL,
    BDR_LEFT_T,      BDR_RIGHT_T,
    BDR_TOP_T,       BDR_BOTTOM_T,
    BDR_CROSS,
} border_t;

// -----------------------------------------------------------------------
// window flags

typedef enum
{
    WIN_BOXED  = 1,         // has a border
    WIN_LOCKED = 2,         // scroll locked
    WIN_FILLED = 4          // backfilled with SOLID character
} win_flags_t;

// -----------------------------------------------------------------------

typedef struct
{
    attribs_t attrs;
    union
    {
        attribs_t selected;
        attribs_t old_attrs;
    };
    union
    {
        attribs_t disabled;
        attribs_t bdr_attrs;
    };
} win_attr_grp_t;

// -----------------------------------------------------------------------

typedef struct
{
    cell_t *buffer;
    void *screen;
    win_flags_t flags;
    int32_t name;           // fnb-1a of window name
    int32_t blank;          // window fill character for backdrop windows
    int16_t width;          // window dimensions
    int16_t height;
    int16_t xco;            // window x/y coordinat within screen
    int16_t yco;
    int16_t cx;             // cursor position within window
    int16_t cy;
    int16_t bdr_type;

    win_attr_grp_t attr_grp;
} window_t;

// -----------------------------------------------------------------------
// visibility hidden

int16_t win_alloc(window_t *win);

// -----------------------------------------------------------------------

API void uC_win_close(window_t *win);
API window_t *uC_win_open(int16_t width, int16_t height);
API void uC_win_pop(window_t *win);
API int16_t uC_win_set_pos(window_t *win, int16_t x, int16_t y);
API void uC_win_erase_line(window_t *win, int16_t line);
API void uC_win_clear(window_t *win);
API void uC_win_scroll_up(window_t *win);
API void uC_win_scroll_dn(window_t *win);
API void uC_win_scroll_lt(window_t *win);
API void uC_win_scroll_rt(window_t *win);
API void uC_win_cup(window_t *win, int16_t x, int16_t y);
API void uC_win_set_cx(window_t *win, int16_t x);
API void uC_win_set_cy(window_t *win, int16_t y);
API void uC_win_crsr_up(window_t *win);
API void uC_win_crsr_dn(window_t *win);
API void uC_win_crsr_lt(window_t *win);
API void uC_win_crsr_rt(window_t *win);
API void uC_win_cr(window_t *win);
API void uC_win_emit(window_t *win, int32_t c);
API cell_t *uC_win_peek_xy(window_t *win, uint16_t x, uint16_t y);
API cell_t *uC_win_peek(window_t *win);
API void uC_win_el(window_t *win);
API void uC_win_set_flag(window_t *win, win_flags_t flag);
API void uC_win_clr_flag(window_t *win, win_flags_t flag);
API void uC_win_set_bold(window_t *win);
API void uC_win_clr_bold(window_t *win);
API void uC_win_set_rev(window_t *win);
API void uC_win_clr_rev(window_t *win);
API void uC_win_set_ul(window_t *win);
API void uC_win_clr_ul(window_t *win);
API void uC_win_set_bdr_bold(window_t *win);
API void uC_win_clr_bdr_bold(window_t *win);
API void uC_win_set_bdr_rev(window_t *win);
API void uC_win_clr_bdr_rev(window_t *win);
API void uC_win_set_bdr_ul(window_t *win);
API void uC_win_clr_bdr_ul(window_t *win);
API void uC_win_set_fg(window_t *win, color_t color);
API void uC_win_set_bg(window_t *win, color_t color);
API void uC_win_set_gray_fg(window_t *win, color_t color);
API void uC_win_set_gray_bg(window_t *win, color_t color);
API void uC_win_set_rgb_fg(window_t *win, color_t r, color_t g, color_t b);
API void uC_win_set_rgb_bg(window_t *win, color_t r, color_t g, color_t b);
API void uC_win_set_bdr_fg(window_t *win, color_t color);
API void uC_win_set_bdr_bg(window_t *win, color_t color);
API void uC_win_set_bdr_gray_fg(window_t *win, color_t color);
API void uC_win_set_bdr_gray_bg(window_t *win, color_t color);
API void uC_win_set_bdr_rgb_fg(window_t *win, color_t r, color_t g, color_t b);
API void uC_win_set_bdr_rgb_bg(window_t *win, color_t r, color_t g, color_t b);

// -----------------------------------------------------------------------

#endif // WINDOW_H

// =======================================================================
