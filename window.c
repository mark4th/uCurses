// window.c
// -----------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#include "h/tui.h"
#include "h/color.h"

// -----------------------------------------------------------------------

void win_pop(window_t *win)
{
  screen_t *scr;

  scr = win_scr_get(win);

  win_detach(win);
  win_attach(scr, win);
}

// -----------------------------------------------------------------------

bool alloc_win(window_t *win)
{
  uint16_t size;
  uint8_t *p;

  size = win_size(win);
  p = malloc(size);

  if(NULL != p)
  {
    win_buff_set(win, (uint32_t *)p);
    return true;
  }
  return false;
}

// -----------------------------------------------------------------------

bool open_window(window_t *win, uint16_t width, uint16_t height)
{
  bool f;

  win_width_set(win, width);
  win_height_set(win,height);
  win_attrs_set(win, WHITE);   // black bg, white fg, no attribs
  win_flags_set(win, 0);

  f = alloc_win(win);

  return f;
}

// -----------------------------------------------------------------------

void close_win(window_t *win)
{
  if(NULL != win_scr_get(win))
  {
    win_detach(win);
  }
  free(win->buffer);
}

// -----------------------------------------------------------------------
// returns xx yy cc where xx = attribs, yy = colors and cc = char

static uint32_t make_char(window_t *win, uint8_t c)
{
  return (win_attrs_get(win) | (c << 8));
}

// -----------------------------------------------------------------------

static uint32_t make_border_char(window_t *win, uint8_t c)
{
  return (win_battrs_get(win) | (c << 8));
}

// -----------------------------------------------------------------------

static uint16_t get_blank(window_t *win)
{
  uint16_t c1;

  c1 = (0 != (win_flags_get(win) & WIN_FILLED))
    ? win_blank_get(win) : 0x20;
}

// -----------------------------------------------------------------------

void scroll_up(window_t *win)
{
  uint16_t i;
  uint32_t *src;
  uint32_t *dst;

  for(i = 0; i < win_height_get(win) - 1; i++)
  {
    dst = to_win_line(win, i);
    src = to_win_line(win, i + 1);
    memcpy(dst, src, win_bpl(win));
  }
  for(i = 0; i < win_width_get(win); i++)
  {
    // blank last line here
  }
}

// =======================================================================
