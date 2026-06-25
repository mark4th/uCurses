// uC_screen.h
// -----------------------------------------------------------------------

#ifndef UC_SCREEN_H
#define UC_SCREEN_H

// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_window.h"
#include "uC_list.h"

// -----------------------------------------------------------------------

typedef struct uC_screen_s
{
    uC_list_t windows;      // linked list of windows
    uC_list_t status;       // linked list of 1 line status windows
    uC_list_t view_groups;  // linked list of widget view groups
    uC_list_t shortcuts;    // screen-level keyboard shortcuts
    void *menu_bar;         // menu_bar_t
    void *backdrop;         // uC_window_t always drawn first
    uC_window_t *popup;     // uC_window_t always drawn last
    uC_window_t *too_small_popup;
    uC_window_t *selected;

    int16_t tab_order;      // current windows tab selection order
    int16_t width;          // screen dimensions
    int16_t height;
    int16_t min_width;      // optional minimum usable dimensions
    int16_t min_height;
    int16_t cx;             // cursor corrdinates within screen
    int16_t cy;
    bool shortcuts_enabled;

    cell_t *buffer1;        // screen buffer 1 and 2
    cell_t *buffer2;
} uC_screen_t;

// -----------------------------------------------------------------------
// visibility hidden

int16_t scr_alloc(uC_screen_t *scr);
void init_backdrop(uC_screen_t *scr, uC_window_t *win);
void scr_draw_win(uC_window_t *win);
int16_t win_chk_pos(uC_window_t *win, uC_screen_t *scr,
    int16_t xco, int16_t yco);

// -----------------------------------------------------------------------

API uC_screen_t *uC_scr_open(int16_t width, int16_t height);
API void uC_scr_close_view_groups(uC_screen_t *scr);
API void uC_scr_close(uC_screen_t *scr);
API void uC_scr_add_backdrop(uC_screen_t *scr);
API void uC_scr_draw_screen(uC_screen_t *scr);
API bool uC_scr_resize_hold(uC_screen_t *scr);
API void uC_scr_set_min_size(uC_screen_t *scr, int16_t width, int16_t height);
API void uC_scr_win_attach(uC_screen_t *scr, uC_window_t *win);
API void uC_scr_win_detach(uC_window_t *win);
API void uC_scr_enable_shortcuts(uC_screen_t *scr);
API void uC_scr_disable_shortcuts(uC_screen_t *scr);
API bool uC_scr_shortcuts_enabled(uC_screen_t *scr);
#ifdef UC_POPUPS
API void uC_scr_popup_attach(uC_screen_t *scr, uC_window_t *win);
API void uC_scr_popup_detach(uC_window_t *win);
#endif
API void uC_scr_win_tab_next(uC_screen_t *scr);

// -----------------------------------------------------------------------

#endif // UC_SCREEN_H

// =======================================================================
