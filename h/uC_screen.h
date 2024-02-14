// screen.h
// -----------------------------------------------------------------------

#ifndef UC_SCREEN_H
#define UC_SCREEN_H

// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_window.h"
#include "uC_list.h"

// -----------------------------------------------------------------------

typedef struct
{
    uC_list_t windows;      // linked list of windows
    void *menu_bar;         // menu_bar_t
    cell_t *buffer1;        // screen buffer 1 and 2
    cell_t *buffer2;
    void *backdrop;         // uC_window_t
    int16_t width;          // screen dimensions
    int16_t height;
    int16_t cx;             // cursor corrdinates within screen
    int16_t cy;
} uC_screen_t;

// -----------------------------------------------------------------------
// visibility hidden

int16_t scr_alloc(uC_screen_t *scr);
void init_backdrop(uC_screen_t *scr, uC_window_t *win);

// -----------------------------------------------------------------------

API uC_screen_t *uC_scr_open(int16_t width, int16_t height);
API void uC_scr_close(uC_screen_t *scr);
API void uC_scr_add_backdrop(uC_screen_t *scr);
API void uC_scr_draw_screen(uC_screen_t *scr);
API void uC_scr_win_attach(uC_screen_t *scr, uC_window_t *win);
API void uC_scr_win_detach(uC_window_t *win);

// -----------------------------------------------------------------------

#endif // UC_SCREEN_H

// =======================================================================
