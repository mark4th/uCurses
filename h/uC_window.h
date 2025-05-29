// window.h
// -----------------------------------------------------------------------

#ifndef UC_WINDOW_H
#define UC_WINDOW_H

#include "uCurses.h"
#include "uC_attribs.h"
#include "uC_list.h"

// -----------------------------------------------------------------------
// structure of each cell of a window / screen

typedef struct
{
    uC_attribs_t attrs;     // bold, underline, gray scale, rgb
    int32_t code;           // utf-8 codepoint
} __attribute__((packed)) cell_t;

// -----------------------------------------------------------------------

enum
{
    SOLID = 0x2592,         // utf-8 'blank' char for backdrop
};

// -----------------------------------------------------------------------
// window flags

typedef enum
{
    WIN_BOXED  = 1 << 0,       // has a border
    WIN_LOCKED = 1 << 1,       // scroll locked
    WIN_FILLED = 1 << 2,       // backfilled with SOLID character
    WIN_HIDDEN = 1 << 3,
    WIN_FOCUS  = 1 << 4
} __attribute__((packed)) win_flags_t;

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
    uint16_t border_type;   // cant include borders.h

    // a window name is only drawn if the window has a border and will
    // be drawn 2 characters to the right of the upper left corner of
    // that border

    char *display_name;

    // if a window has any widget view groups attached to it they will be
    // drawn when the window is drawn

    uC_list_t widget_view_groups;

    uC_attribs_t attrs;          // attribs for stuff drawn in window
    uC_attribs_t bdr_attrs;      // normal attribs for window border
    uC_attribs_t focus_attrs;    // focussed attribs for window border
} uC_window_t;

// -----------------------------------------------------------------------
// visibility hidden

int16_t win_alloc(uC_window_t *win);

// -----------------------------------------------------------------------

API void uC_win_close(uC_window_t *win);
API uC_window_t *uC_win_open(int16_t width, int16_t height);
API void uC_win_pop(uC_window_t *win);
API int16_t uC_win_set_pos(uC_window_t *win, int16_t x, int16_t y);
API void uC_win_clear_line(uC_window_t *win, int16_t line);
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
API void uC_win_set_fg(uC_window_t *win, uC_color_t color);
API void uC_win_set_bg(uC_window_t *win, uC_color_t color);
API void uC_win_set_gray_fg(uC_window_t *win, uC_color_t color);
API void uC_win_set_gray_bg(uC_window_t *win, uC_color_t color);
API void uC_win_set_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_win_set_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_win_set_bdr_fg(uC_window_t *win, uC_color_t color);
API void uC_win_set_bdr_bg(uC_window_t *win, uC_color_t color);
API void uC_win_set_bdr_gray_fg(uC_window_t *win, uC_color_t color);
API void uC_win_set_bdr_gray_bg(uC_window_t *win, uC_color_t color);
API void uC_win_set_bdr_rgb_fg(uC_window_t *win, uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_win_set_bdr_rgb_bg(uC_window_t *win, uC_color_t r, uC_color_t g, uC_color_t b);
API void uC_win_set_border(uC_window_t *win, uint16_t border_type);

// -----------------------------------------------------------------------

#endif // UC_WINDOW_H

// =======================================================================
