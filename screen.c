// screen.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "h/list.h"
#include "h/tui.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

fp_t draw_window;           // usually void _draw_window(window_t *w1)

extern uint16_t width;
extern uint16_t height;
extern uint16_t cx;
extern uint16_t cy;
extern uint16_t attr;

static void noop(void *x){;}

// -----------------------------------------------------------------------
// attach window to screen

void win_attach(screen_t *s, window_t *w)
{
  w->screen = s;

  list_append(s->windows, (void *)w);
}

// -----------------------------------------------------------------------

void win_detach(window_t *w)
{
  screen_t *s;

  s = w->screen;
  list_remove(s->windows, (void *)w);

  w->screen = NULL;
}

// -----------------------------------------------------------------------
// return number of character cells in screen

uint32_t scr_size(screen_t *scr)
{
  return (scr->width * scr->height);
}

// -----------------------------------------------------------------------

uint16_t scr_bpl(screen_t *scr)
{
  return (scr->width * CELL);
}

// -----------------------------------------------------------------------

uint32_t *scr_line(screen_t *scr, uint16_t line)
{
  uint16_t bpl;

  bpl = scr_bpl(scr);
  return((scr->buffer1) + (bpl * line));
}

// -----------------------------------------------------------------------

bool alloc_screen(screen_t *scr)
{
  uint32_t *p;
  uint32_t s;

  s = scr_size(scr);        // allocate 2 buffers at once
  p = (uint32_t *)malloc(s * CELL * 2); 

  if(NULL == p)
  {
    return false;
  }

  scr->buffer1 = p;
  scr->buffer2 = (p + s * CELL);

  return true;
}

// -----------------------------------------------------------------------

bool open_screen(screen_t *s, uint16_t width, uint16_t height)
{
  bool f;

  s->width = width;
  s->height = height;
  f = alloc_screen(s);

  return f;
}

// -----------------------------------------------------------------------

void close_screen(screen_t *s)
{
  free(s->buffer1);
}

// -----------------------------------------------------------------------

void _draw_window(window_t *w1)
{
  printf("foo\n");
}

// -----------------------------------------------------------------------

void draw_windows(screen_t *s)
{
  window_t *w;
  node_t *n1;

  n1 = (node_t *)s->windows;

  while(NULL != n1)
  {
    w = (window_t *) n1->payload;
    (draw_window)(w);
    n1 = n1->next;
  }
}

// -----------------------------------------------------------------------
// erases buffer 1 of screen

void clear_screen(screen_t *s)
{
  uint16_t size;
  uint32_t *p;

  size = (s->width * s->height);
  p = (uint32_t *)s->buffer1;

  while(size--)
  {
    *p++ = 0x07020;
  }
}

// -----------------------------------------------------------------------

void refresh_screen(screen_t *s)
{
  uint16_t size;
  uint32_t *p;

  size = (s->width * s->height);
  p = (uint32_t *) s->buffer2;

  while(size--)
  {
    *p++ = 0;
  }
}

// -----------------------------------------------------------------------
// copy char at index in buffer1 to buffer2, output to uCurses buffer

void screen_c2$(uint32_t ix, screen_t *s)
{
  uint32_t c;
  uint16_t w;
  uint16_t x, y;

  w = s->width;
  c = s->buffer1[ix];
  s->buffer2[ix] = c;

  y = ix / w;
  x = ix % w;

  if((cx != x) && (cy != y))
  {
    cup(x, y);
    cx = x;
    cy = y;
  }
  c2$(c & 0xff);
  cx++;
}

// -----------------------------------------------------------------------

void _draw_screen(screen_t *s)
{
  uint32_t *p1;
  uint32_t *p2;
  uint16_t size;
  uint16_t attr_save;
  uint16_t ix, jx;
  uint16_t a;

  p1 = s->buffer1;
  p2 = s->buffer2;

  attr_save = attr;
  size = scr_size(s);

  send$ = &noop;          // initialize function pointers
  draw_windows(s);

  for(ix = 0; ix < size; ix++)
  {
    a = p1[ix] >> 8;        // does char need updating?
    if(a != (p2[ix] >> 8))  // are buffer1 and buffer2 different
    {
      attr = a;             // yes, output escape sequences for 
      set_attribs();        // selected attributes / colors

      // output all other chars from this index on with same attribs

      jx = ix;

      while(jx != size)     
      {
        if((a == (p1[jx] >> 8)) && (a != (p2[jx] >> 8)))
        {
          p2[jx] = p1[jx];
          screen_c2$(jx, s);
        }
        jx++;
      }
    }
  }
  attr = attr_save;
  set_attribs();
  send$ = &_send$;          // initialize function pointers
  _send$(NULL);
}

// -----------------------------------------------------------------------

// =======================================================================
