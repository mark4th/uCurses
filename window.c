// window.c
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "h/color.h"
#include "h/tui.h"

// -----------------------------------------------------------------------

uint16_t win_bpl(window_t *win)
{
  return(win->width * CELL);
}

// -----------------------------------------------------------------------

uint32_t win_attrs(window_t *w uint8_t c)
{
  uint32_t a;

  a  = w->attr << 16;
  a |= w>colors << 8;

  return a | c;
}

// -----------------------------------------------------------------------

uint32_t win_blank(window_t *w)
{
  uint32_t blank;
}

// -----------------------------------------------------------------------

uint32_t win_size(window_t *win)
{
  uint16_t bpl;
  
  bpl = win_bpl(win);

  return(win->height * bpl);
}

// -----------------------------------------------------------------------

uint32_t *win_line(window_t *win, uint16_t line)
{
  uint16_t bpl;

  bpl = win_bpl(win);

  return((win->buffer) + (line * bpl));
}

// -----------------------------------------------------------------------

void win_pos(window_t *win, uint16_t x, uint16_t y)
{
  win->x = x;
  win->y = y;
}

// -----------------------------------------------------------------------

void win_at(window_t *win, uint16_t x, uint16_t y)
{
  win->cx = x;
  win->cy = y;
}

// -----------------------------------------------------------------------

void win_fg(window_t *win, uint8_t fg)
{
  uint8_t color;
  color = win->colors;
  color &= 0x0f;
  color |= fg;
  win->colors = color;
}

// -----------------------------------------------------------------------

void win_bg(window_t *win, uint8_t bg)
{
  uint8_t color;

  color = win-> colors;
  color &= 0x0f;
  color |= (bg << 4);
  win->colors = color;
}

// -----------------------------------------------------------------------

void win_setAttr(window_t *win, uint8_t attr)
{
  win->attr |= attr;
}

// -----------------------------------------------------------------------

void win_clrAttr(window_t *win, uint8_t attr)
{
  win->attr &= ~attr;
}

// -----------------------------------------------------------------------

void win_pop(window_t *w)
{
  screen_t *s;

  s = w->screen;

  win_detach(w);
  win_attach(s, w);
}

// -----------------------------------------------------------------------

bool alloc_win(window_t *w)
{
  uint16_t size;
  uint8_t *p;

  size = win_size(w);
  p = malloc(size);
  if(NULL != p)
  {
    w->buffer = (uint32_t *)p;
    return true;
  }
  return false;
}

// -----------------------------------------------------------------------

bool open_window(window_t *w,uint16_t width, uint16_t height)
{
  bool f;

  w->width = width;
  w->height = height;
  w->colors = WHITE;
  w->attr = 0;
  w->flags = 0;

  f = alloc_win(w);

  return f;
}

// -----------------------------------------------------------------------

void close_win(window_t *w)
{
  if(NULL != w->screen)
  {
    win_detach(w);
  }
  free(w->buffer);
}

// -----------------------------------------------------------------------
// returns xx yy cc where xx = attribs, yy = colors and cc = char

static uint32_t make_char(window_t *w, uint8_t c)
{
  uint32_t n1;

  n1 |= c;
  n1 |= (w->attr  << 16);
  n1 |= (w->colors << 8);

  return n1;
}

// -----------------------------------------------------------------------

static uint32_t make_border_char(window_t *w, uint8_t c)
{
  uint32_t n1;

  n1 |= c;
  n1 |= (w->battr << 16);
  n1 |= (w->bcolors << 8);

  return n1;
}

// -----------------------------------------------------------------------

static uint32_t get_blank(window_t *w)
{
  uint8_t c1;

  c1 = (0 != (w->flags & filled))
    ? w->blank : 0x20;
}

// -----------------------------------------------------------------------

void scroll_up(window_t *w)
{
  uint16_t i;
  uint32_t *src;
  uint32_t *dst;

  for(i = 0; i < w->height - 1; i++)
  {
    dst = win_line(i);
    src = win_line(i + 1);
    memcpy(dst, src, win_bpl(w));
  }
  for(i = 0; i < w->width; i++)
  {

  }
}

// =======================================================================
