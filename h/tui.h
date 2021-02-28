// tui.h
// -----------------------------------------------------------------------

#pragma once

// -----------------------------------------------------------------------

#include <inttypes.h>

#include "list.h"

// -----------------------------------------------------------------------
// structure of each cell of a window / screen

typedef struct
{
    uint8_t attrs[8];       // bold, blink, underline, gray scale, rgb
    uint32_t code;           // utf-8 codepoint
} cell_t;

// -----------------------------------------------------------------------

typedef struct
{
    list_t *windows;        // linked list of windows
    list_t *menus;          // todo
    cell_t *buffer1;        // screen buffer 1 and 2
    cell_t *buffer2;
    uint16_t width;         // screen dimensions
    uint16_t height;
    uint16_t cx;            // cursor corrdinates within screen
    uint16_t cy;
} screen_t;

// -----------------------------------------------------------------------

typedef enum
{
    WIN_BOXED      = 1,     // has a border
    WIN_LOCKED     = 2,     // scroll locked
    WIN_FILLED     = 4      // checkerboard char filled
} win_flags_t;

// -----------------------------------------------------------------------

typedef struct
{
    win_flags_t flags;
    node_t      *links;
    cell_t      *buffer;
    screen_t    *screen;
    uint32_t    blank;      // window fill character for backdrop windows
    uint16_t    width;      // window dimensions
    uint16_t    height;
    uint16_t    xco;        // window x/y coordinat within screen
    uint16_t    yco;
    uint16_t    cx;         // cursor position within window
    uint16_t    cy;
    uint8_t     attrs[8];      // bold blink underline, gray scale, rgb etc
    uint8_t     old_attrs[8];  // previous state..
    uint8_t     b_attrs[8];    // likewise for the windows border if it has
    uint8_t     b_old_attrs[8];
} window_t;

// -----------------------------------------------------------------------

void scr_win_attach(screen_t *scr, window_t *win);
void scr_win_detach(window_t *win);

void win_draw(window_t *win);


// =======================================================================
