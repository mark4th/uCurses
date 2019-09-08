// tui.h
// -----------------------------------------------------------------------

#pragma once

// -----------------------------------------------------------------------

#include <inttypes.h>

#include "list.h"

// -----------------------------------------------------------------------

#define CELL sizeof(uint32_t)

// -----------------------------------------------------------------------

#define KEEP __attribute__((__used__))

// -----------------------------------------------------------------------

typedef enum
{
  WIN_BOXED      = 1,       // has a border
  WIN_LOCKED     = 2,       // scroll locked
  WIN_FILLED     = 4        // checkerboard char filled
} win_flag_t;

// -----------------------------------------------------------------------

typedef struct
{
  list_t *windows;          // linked list of windows
  list_t *menus;            // todo
  uint32_t *buffer1;        // screen buffer 1 and 2
  uint32_t *buffer2;
  uint16_t width;           // screen dimensions
  uint16_t height;
  uint16_t flags;           // not defined yet
  uint16_t reserved;
} screen_t;

// -----------------------------------------------------------------------

typedef struct
{
  screen_t *screen;
  uint32_t *buffer;
  win_flag_t flags;
  uint16_t xco;
  uint16_t yco;
  uint16_t width;
  uint16_t height;
  uint16_t cx;
  uint16_t cy;
  uint16_t blank;
  uint32_t attrs;
  uint16_t battrs;
  uint16_t reserved;
} window_t;

// -----------------------------------------------------------------------

uint16_t scr_width_get(screen_t *scr);
uint16_t scr_height_get(screen_t *scr);
uint32_t *scr_b1_get(screen_t *scr);
uint32_t *scr_b2_get(screen_t *scr);
list_t *scr_win_get(screen_t *scr);

void scr_width_set(screen_t *scr, uint16_t width);
void scr_height_set(screen_t *scr, uint16_t height);
void scr_b1_set(screen_t *scr, uint32_t *b1);
void scr_b2_set(screen_t *scr, uint32_t *b2);
void scr_win_add(screen_t *scr, window_t *win);

uint32_t scr_size(screen_t *scr);
uint16_t scr_bpl(screen_t *scr);
uint32_t *to_scr_line(screen_t *scr, uint16_t line);

uint16_t win_width_get(window_t *win);
uint16_t win_height_get(window_t *win);
uint16_t win_xco_get(window_t *win);
uint16_t win_yco_get(window_t *win);
uint16_t win_cx_get(window_t *win);
uint16_t win_cy_get(window_t *win);
win_flag_t win_flags_get(window_t *win);
uint32_t *win_buff_get(window_t *win);
screen_t *win_scr_get(window_t *win);
uint16_t win_blank_get(window_t *win);
uint32_t win_attrs_get(window_t *win);
uint32_t win_battrs_get(window_t *win);
void win_width_set(window_t *win, uint16_t width);
void win_height_set(window_t *win, uint16_t height);
void win_xco_set(window_t *win, uint16_t xco);
void win_yco_set(window_t *win, uint16_t yco);
void win_cx_set(window_t *win, uint16_t cx);
void win_cy_set(window_t *win, uint16_t cy);
void win_flags_set(window_t *win, uint16_t flags);
void win_buff_set(window_t *win, uint32_t *buffer);
void win_scr_set(window_t *win, screen_t *scr);
void win_blank_set(window_t *win, uint16_t blank);
void win_attrs_set(window_t *win, uint32_t attr);
void win_battrs_set(window_t *win, uint32_t battr);
uint32_t win_bpl(window_t *win);
uint32_t *to_win_line(window_t *win, uint16_t line);
uint32_t win_size(window_t *win);
void win_pos_set(window_t *win, uint16_t xco, uint16_t yco);
void win_at(window_t *win, uint16_t cx, uint16_t cy);
void win_fg_set(window_t *win, uint8_t fg);
void win_bg_set(window_t *win, uint8_t bg);
void win_attr_set(window_t *win, uint32_t attr);
void win_attr_clr(window_t *win, uint32_t attr);
void win_so_set(window_t *win);
void win_so_clr(window_t *win);
void win_ul_set(window_t *win);
void win_ul_clr(window_t *win);
void win_rev_set(window_t *win);
void win_rev_clr(window_t *win);
void win_bold_set(window_t *win);
void win_bold_clr(window_t *win);
void win_alt_set(window_t *win);
void win_alt_clr(window_t *win);
void win_flag_clr(window_t *win, win_flag_t flag);
void win_borders_on(window_t *win);
void win_borders_off(window_t *win);
void win_filled_on(window_t *win);
void win_filled_off(window_t *win);
void win_lock(window_t *win);
void win_unlock(window_t *win);
bool win_is_boxed(window_t *win);
bool win_is_locked(window_t *win);
bool win_is_filled(window_t *win);

// -----------------------------------------------------------------------

void win_attach(screen_t *s, window_t *w);
void win_detach(window_t *w);

// =======================================================================
