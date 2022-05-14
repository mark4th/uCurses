// screen.h
// -----------------------------------------------------------------------

#ifndef SCREEN_H
#define SCREEN_H

// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_window.h"
#include "uC_list.h"

// -----------------------------------------------------------------------

typedef struct
{
    list_t windows;         // linked list of windows
    void *menu_bar;         // menu_bar_t
    cell_t *buffer1;        // screen buffer 1 and 2
    cell_t *buffer2;
    void *backdrop;         // window_t
    int16_t width;          // screen dimensions
    int16_t height;
    int16_t cx;             // cursor corrdinates within screen
    int16_t cy;
} screen_t;

// -----------------------------------------------------------------------
// visibility hidden

int16_t scr_alloc(screen_t *scr);
void init_backdrop(screen_t *scr, window_t *win);

// -----------------------------------------------------------------------

API screen_t *uC_scr_open(int16_t width, int16_t height);
API void uC_scr_close(screen_t *scr);
API void uC_scr_add_backdrop(screen_t *scr);
API void uC_scr_draw_screen(screen_t *scr);
API void uC_scr_win_attach(screen_t *scr, window_t *win);
API void uC_scr_win_detach(window_t *win);

// -----------------------------------------------------------------------

#endif // SCREEN_H

// =======================================================================
