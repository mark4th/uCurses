// tui.h
// -----------------------------------------------------------------------

    #pragma once

// -----------------------------------------------------------------------

#include <inttypes.h>

#include "color.h"
#include "list.h"
#include "border.h"

// -----------------------------------------------------------------------
// utf-8 codepoint for 'blank' char used in screen backdrop windows

    #define SOLID  0x2592
    #define SHADOW 0x2591

// -----------------------------------------------------------------------
// structure of each cell of a window / screen

typedef struct
{
    uint8_t attrs[8];       // bold, blink, underline, gray scale, rgb
    uint32_t code;          // utf-8 codepoint
} cell_t;

// -----------------------------------------------------------------------
// window flags

typedef enum
{
    WIN_BOXED      = 1,     // has a border
    WIN_LOCKED     = 2,     // scroll locked
    WIN_FILLED     = 4      // backfilled with SOLID character
} win_flags_t;

// -----------------------------------------------------------------------

typedef void (*menu_fp_t)(void);

// -----------------------------------------------------------------------

typedef struct
{
    node_t *links;
    char *name;
    menu_fp_t fp;           // function to execute
    uint16_t shortcut;      // keyboard shortcut
} menu_item_t;

// -----------------------------------------------------------------------

typedef struct
{
    char *name;             // menu bar name for this pulldown menu
    uint16_t width;         // width of widest item in pulldown menu
    list_t items;           // list of items in this pulldown
    uint8_t attr[8];        // attribs for pulldown menu border
    uint16_t flags;         // masks for enabled/disabled etc
    uint16_t which;         // current selected item
    uint16_t xco;           // x coordinate of menu window
    uint8_t normal[8];      // attribs for non selected menu items
    uint8_t selected[8];    // atrribs for selected menu item
    uint8_t disabled[8];    // attribs for disabled meny items
} pulldown_t;

// -----------------------------------------------------------------------

typedef struct
{
    list_t *items;            // list of pulldown menus in this bar
    void *window;             // fwd ref to window_t * grrr (c sucks)
    uint16_t xco;             // x coordinate of next pulldown
    uint16_t which;           // which pulldown item is active
    uint8_t attr_normal[8];   // attribs for non selected menu bar items
    uint8_t attr_selected[8]; // attribs for selected menu bar items
    uint8_t attr_disabled[8]; // attribs for disabled menu bar items
} menu_bar_t;

// -----------------------------------------------------------------------

typedef struct
{
    list_t windows;         // linked list of windows
    menu_bar_t *menu_bar;
    cell_t *buffer1;        // screen buffer 1 and 2
    cell_t *buffer2;
    void *backdrop;         // really a window_t *
    uint16_t width;         // screen dimensions
    uint16_t height;
    uint16_t cx;            // cursor corrdinates within screen
    uint16_t cy;
} screen_t;

// -----------------------------------------------------------------------

typedef struct
{
    node_t      *links;
    cell_t      *buffer;
    screen_t    *screen;
    win_flags_t flags;
    uint32_t    blank;         // window fill character for backdrop windows
    uint16_t    width;         // window dimensions
    uint16_t    height;
    uint16_t    xco;           // window x/y coordinat within screen
    uint16_t    yco;
    uint16_t    cx;            // cursor position within window
    uint16_t    cy;
    uint16_t    bdr_type;
    uint8_t     attrs[8];      // bold blink underline, gray scale, rgb etc
    uint8_t     old_attrs[8];  // previous state..
    uint8_t     bdr_attrs[8];  // likewise for the windows border if it has
} window_t;

// -----------------------------------------------------------------------

#define win_clr_attr(win, attr) win->attrs[ATTR] &= ~attr
#define win_set_ul(win)    win_set_attr(win, UNDERLINE)
#define win_set_rev(win)   win_set_attr(win, REVERSE)
#define win_set_bold(win)  win_set_attr(win, BOLD)
#define win_set_blink(win) win_set_attr(win, BLINK)
#define win_clr_ul(win)    win_clr_attr(win, UNDERLINE)
#define win_clr_rev(win)   win_clr_attr(win, REVERSE)
#define win_clr_bold(win)  win_clr_attr(win, BOLD)
#define win_clr_blink(win) win_clr_attr(win, BLINK)

// -----------------------------------------------------------------------

#define win_set_boxed(win)   win->flags |= WIN_BOXED
#define win_set_locked(win)  win->flags |= WIN_LOCKED
#define win_set_filled(win)  win->flags |= WIN_FILLED

#define win_clr_boxed(win)   win->flags &= ~WIN_BOXED
#define win_clr_locked(win)  win->flags &= ~WIN_LOCKED
#define win_clr_filled(win)  win->flags &= ~WIN_FILLED

// -----------------------------------------------------------------------

void win_pop(window_t *win);
void win_close(window_t *win);
window_t *win_open(uint16_t width, uint16_t height);
uint16_t win_set_pos(window_t *win, uint16_t x, uint16_t y);
void win_set_gray_fg(window_t *win, uint8_t c);
void win_set_gray_bg(window_t *win, uint8_t c);
void win_set_rgb_fg(window_t *win, uint8_t r, uint8_t g, uint8_t b);
void win_set_rgb_bg(window_t *win, uint8_t r, uint8_t g, uint8_t b);
void win_set_fg(window_t *win, uint8_t color);
void win_set_bg(window_t *win, uint8_t color);
void win_scroll_up(window_t *win);
void win_scroll_dn(window_t *win);
void win_scroll_lt(window_t *win);
void win_scroll_rt(window_t *win);
void win_cup(window_t *win, uint16_t x, uint16_t y);
void win_set_cx(window_t *win, uint16_t x);
void win_set_cy(window_t *win, uint16_t y);
void win_crsr_up(window_t *win);
void win_crsr_dn(window_t *win);
void win_crsr_lt(window_t *win);
void win_crsr_rt(window_t *win);
void win_cr(window_t *win);
void win_emit(window_t *win, uint32_t c);
void win_clear(window_t *win);
void win_draw_borders(window_t *win);
void win_el(window_t *win);

// -----------------------------------------------------------------------

void scr_win_attach(screen_t *scr, window_t *win);
void scr_win_detach(window_t *win);
screen_t *scr_open(uint16_t width, uint16_t height);
void scr_close(screen_t *scr);
void scr_cup(screen_t *scr, uint16_t x, uint16_t y);
void scr_do_draw_screen(screen_t *scr);
void scr_add_backdrop(screen_t *scr);

// =======================================================================
