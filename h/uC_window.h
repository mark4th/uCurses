// uC_window.h
// -----------------------------------------------------------------------

#ifndef UC_WINDOW_H
#define UC_WINDOW_H

// -----------------------------------------------------------------------
// in every include file I have the include guards FIRST.  If I put them
// above these includes the build fails and I do not understand why

#include "uCurses.h"
#include "uC_attribs.h"
#include "uC_list.h"

// -----------------------------------------------------------------------
// structure of each cell of a window / screen

typedef struct
{
    uC_attribs_t attrs;     // bold, underline, gray scale, rgb
    uint32_t code;           // utf-8 codepoint
} cell_t;

// -----------------------------------------------------------------------

// enum
// {
//     uC_SOLID = 0x2592,      // utf-8 'blank' char for backdrop
// } __attribute__((__packed__));

// -----------------------------------------------------------------------
// window flags

typedef enum
{
    // bit positions
    uC_WIN_BIT_BOXED  = 0,
    uC_WIN_BIT_LOCKED = 1,
    uC_WIN_BIT_NAMED  = 2,
    uC_WIN_BIT_FILLED = 3,
    uC_WIN_BIT_FOCUS  = 4,

    // bit masks
    uC_WIN_BOXED    = 1 << uC_WIN_BIT_BOXED,   // has a border
    uC_WIN_LOCKED   = 1 << uC_WIN_BIT_LOCKED,  // scroll locked
    uC_WIN_NAMED    = 1 << uC_WIN_BIT_NAMED,   // window has a name
    uC_WIN_FILLED   = 1 << uC_WIN_BIT_FILLED,  // backfilled with SOLID character
    uC_WIN_FOCUS    = 1 << uC_WIN_BIT_FOCUS,   // window has focus
} __attribute__((packed)) win_flags_t;

// -----------------------------------------------------------------------

typedef struct
{
    cell_t *buffer;
    void *screen;
    win_flags_t flags;
    int32_t name;           // fnv-1a of window name
    int32_t blank;          // window fill character

    int16_t width;          // window dimensions
    int16_t height;
    int16_t xco;            // window x/y coordinat within screen
    int16_t yco;
    int16_t tab_order;
    int16_t cx;             // cursor position within window
    int16_t cy;

    uint16_t border_type;   // cant include borders.h (circular deps)

    // a window name is only drawn if the window has a border and will
    // be drawn 2 characters to the right of the upper left corner of
    // that border

    const char *display_name;

    uC_attribs_t bdr_attrs;      // normal attribs for window border
    uC_attribs_t focus_attrs;    // focussed attribs for window border
    uC_attribs_t attrs;          // attribs for stuff drawn in window
} uC_window_t;

// -----------------------------------------------------------------------
// visibility hidden

int16_t win_alloc(uC_window_t *win);

// -----------------------------------------------------------------------

API uC_window_t *uC_win_open(int16_t width, int16_t height);
API void uC_win_close(uC_window_t *win);
API int16_t uC_win_set_pos(uC_window_t *win, int16_t x, int16_t y);
API void uC_win_clear_line(uC_window_t *win, int16_t line);
API void uC_win_clear(uC_window_t *win);
API void uC_win_copy_win(uC_window_t *dst, uC_window_t *src);
API void uC_win_scroll_up(uC_window_t *win);
API void uC_win_scroll_dn(uC_window_t *win);
API void uC_win_scroll_lt(uC_window_t *win);
API void uC_win_scroll_rt(uC_window_t *win);
API void uC_win_scroll_up_n(uC_window_t *win, int16_t n);
API void uC_win_scroll_dn_n(uC_window_t *win, int16_t n);
API void uC_win_scroll_lt_n(uC_window_t *win, int16_t n);
API void uC_win_scroll_rt_n(uC_window_t *win, int16_t n);
API void uC_win_cup(uC_window_t *win, int16_t x, int16_t y);
API void uC_win_set_cx(uC_window_t *win, int16_t x);
API void uC_win_set_cy(uC_window_t *win, int16_t y);
API void uC_win_crsr_up(uC_window_t *win);
API void uC_win_crsr_dn(uC_window_t *win);
API void uC_win_crsr_lt(uC_window_t *win);
API void uC_win_crsr_rt(uC_window_t *win);
API void uC_win_cr(uC_window_t *win);
API void uC_win_emit(uC_window_t *win, uint32_t c);
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
API void uC_win_set_fg(uC_window_t *win, uC_color_t color);
API void uC_win_set_bg(uC_window_t *win, uC_color_t color);
API void uC_win_set_gray_fg(uC_window_t *win, uC_colors_gray_t color);
API void uC_win_set_gray_bg(uC_window_t *win, uC_colors_gray_t color);
API void uC_win_set_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_win_set_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_win_set_bdr_fg(uC_window_t *win, uC_color_t color);
API void uC_win_set_bdr_bg(uC_window_t *win, uC_color_t color);
API void uC_win_set_bdr_gray_fg(uC_window_t *win, uC_colors_gray_t color);
API void uC_win_set_bdr_gray_bg(uC_window_t *win, uC_colors_gray_t color);
API void uC_win_set_bdr_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_win_set_bdr_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_win_set_border(uC_window_t *win, uint16_t border_type,
    uC_attribs_t bdr_attrs, uC_attribs_t focus_attrs);
API void uC_win_set_name(uC_window_t *win, const char *name);
API void uC_win_set_focus(uC_window_t *win);

// -----------------------------------------------------------------------

#endif // UC_WINDOW_H

// =======================================================================
