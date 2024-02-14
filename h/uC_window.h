// window.h
// -----------------------------------------------------------------------

#ifndef UC_WINDOW_H
#define UC_WINDOW_H

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
    int32_t name;           // fnv-1a of window name
    int32_t blank;          // window fill character for backdrop windows
    int16_t width;          // window dimensions
    int16_t height;
    int16_t xco;            // window x/y coordinat within screen
    int16_t yco;
    int16_t cx;             // cursor position within window
    int16_t cy;
    int16_t bdr_type;

    win_attr_grp_t attr_grp;
} uC_window_t;

// -----------------------------------------------------------------------
// visibility hidden

int16_t win_alloc(uC_window_t *win);

// -----------------------------------------------------------------------

API void uC_win_close(uC_window_t *win);
API uC_window_t *uC_win_open(int16_t width, int16_t height);
API void uC_win_pop(uC_window_t *win);
API int16_t uC_win_set_pos(uC_window_t *win, int16_t x, int16_t y);
API void uC_win_erase_line(uC_window_t *win, int16_t line);
API void uC_win_clear(uC_window_t *win);
API void uC_win_scroll_up(uC_window_t *win);
API void uC_win_scroll_dn(uC_window_t *win);
API void uC_win_scroll_lt(uC_window_t *win);
API void uC_win_scroll_rt(uC_window_t *win);
API void uC_win_cup(uC_window_t *win, int16_t x, int16_t y);
API void uC_win_set_cx(uC_window_t *win, int16_t x);
API void uC_win_set_cy(uC_window_t *win, int16_t y);
API void uC_win_crsr_up(uC_window_t *win);
API void uC_win_crsr_dn(uC_window_t *win);
API void uC_win_crsr_lt(uC_window_t *win);
API void uC_win_crsr_rt(uC_window_t *win);
API void uC_win_cr(uC_window_t *win);
API void uC_win_emit(uC_window_t *win, int32_t c);
API cell_t *uC_win_peek_xy(uC_window_t *win, int16_t x, int16_t y);
API cell_t *uC_win_peek(uC_window_t *win);
API void uC_win_el(uC_window_t *win);
API void uC_win_set_flag(uC_window_t *win, win_flags_t flag);
API void uC_win_clr_flag(uC_window_t *win, win_flags_t flag);
API void uC_win_set_bold(uC_window_t *win);
API void uC_win_clr_bold(uC_window_t *win);
API void uC_win_set_rev(uC_window_t *win);
API void uC_win_clr_rev(uC_window_t *win);
API void uC_win_set_ul(uC_window_t *win);
API void uC_win_clr_ul(uC_window_t *win);
API void uC_win_set_bdr_bold(uC_window_t *win);
API void uC_win_clr_bdr_bold(uC_window_t *win);
API void uC_win_set_bdr_rev(uC_window_t *win);
API void uC_win_clr_bdr_rev(uC_window_t *win);
API void uC_win_set_bdr_ul(uC_window_t *win);
API void uC_win_clr_bdr_ul(uC_window_t *win);
API void uC_win_set_fg(uC_window_t *win, color_t color);
API void uC_win_set_bg(uC_window_t *win, color_t color);
API void uC_win_set_gray_fg(uC_window_t *win, color_t color);
API void uC_win_set_gray_bg(uC_window_t *win, color_t color);
API void uC_win_set_rgb_fg(uC_window_t *win, color_t r, color_t g, color_t b);
API void uC_win_set_rgb_bg(uC_window_t *win, color_t r, color_t g, color_t b);
API void uC_win_set_bdr_fg(uC_window_t *win, color_t color);
API void uC_win_set_bdr_bg(uC_window_t *win, color_t color);
API void uC_win_set_bdr_gray_fg(uC_window_t *win, color_t color);
API void uC_win_set_bdr_gray_bg(uC_window_t *win, color_t color);
API void uC_win_set_bdr_rgb_fg(uC_window_t *win, color_t r, color_t g, color_t b);
API void uC_win_set_bdr_rgb_bg(uC_window_t *win, color_t r, color_t g, color_t b);

// -----------------------------------------------------------------------

#endif // UC_WINDOW_H

// =======================================================================
