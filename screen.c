// screen.c   - uCurses text user interface screen handling
// -----------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#include "h/uCurses.h"
#include "h/tui.h"
#include "h/list.h"

// -----------------------------------------------------------------------

fp_t draw_window;           // usually void _draw_window(window_t *w1)

extern uint16_t attr;

// -----------------------------------------------------------------------

static void noop(void *x){;}

// -----------------------------------------------------------------------
// attach window to screen

void win_attach(screen_t *scr, window_t *win)
{
  win_scr_set(win, scr);
  scr_win_add(scr, win);
}

// -----------------------------------------------------------------------

void win_detach(window_t *win)
{
  screen_t *scr;

  list_t *l = scr_win_get(scr);
  list_remove(l, (void *)win);

  win->screen = NULL;
}

// -----------------------------------------------------------------------

bool alloc_screen(screen_t *scr)
{
  uint32_t *p;
  uint32_t size;

  bool result = false;

  // allocate 2 buffers at once

  size = scr_size(scr);

  p = (uint32_t *)malloc(size * sizeof(uint32_t) * 2);

  if(NULL != p)
  {
    scr_b1_set(scr, p);
    scr_b2_set(scr, p + size * sizeof(uint32_t));

    result = true;
  }

  return result;
}

// -----------------------------------------------------------------------

bool open_screen(screen_t *scr, uint16_t width, uint16_t height)
{
  scr_width_set(scr, width);
  scr_height_set(scr, height);

  return alloc_screen(scr);
}

// -----------------------------------------------------------------------

void close_screen(screen_t *scr)
{
  free((void *)scr_b1_get(scr));
}

// -----------------------------------------------------------------------

void _draw_window(window_t *w1)
{
  printf("foo\n");
}

// -----------------------------------------------------------------------

void draw_windows(screen_t *scr)
{
  window_t *win;
  node_t *n1;

  n1 = (node_t *)scr->windows;

  while(NULL != n1)
  {
    win = (window_t *) n1->payload;
    (draw_window)(win);
    n1 = n1->next;
  }
}

// -----------------------------------------------------------------------
// erases buffer 1 of screen

void clear_screen(screen_t *scr)
{
  uint16_t size;
  uint32_t *p;

  size = scr_size(scr);
  p    = scr_b1_get(scr);

  for( ;size; *p++ = 0x000007020, size--);
}

// -----------------------------------------------------------------------

void refresh_screen(screen_t *scr)
{
  uint16_t size;
  uint32_t *p;

  size = scr_size(scr);
  p    = scr_b1_get(scr);

  for( ;size; *p++ = 0, size--);
}

// -----------------------------------------------------------------------
// copy char at index in buffer1 to buffer2, output to uCurses buffer

void scr_emit(screen_t *scr, uint32_t ix)
{
  uint32_t c;
  uint16_t width;
  uint16_t x, y;
  uint16_t cx, cy;
  uint32_t *p1, *p2;

  p1 = scr_b1_get(scr);
  p2 = scr_b2_get(scr);

  width = scr_width_get(scr);
  c = p1[ix];
  p2[ix] = c;

  y = ix / width;
  x = ix % width;

  if((cx != x) && (cy != y))
  {
    cup(x, y);
  }
  c_emit(c & 0xff);
  cx++;
}

// -----------------------------------------------------------------------

void _draw_screen(screen_t *scr)
{
  uint32_t *p1, *p2;
  uint16_t ix, jx;
  uint16_t attr_save;
  uint16_t size;
  uint16_t a;

  p1 = scr_b1_get(scr);
  p2 = scr_b2_get(scr);

  attr_save = attr;
  size = scr_size(scr);

  send$ = &noop;
  draw_windows(scr);

  for(ix = 0; ix < size; ix++)
  {
    a = p1[ix] >> 8;        // does char need updating?

    if(a != (p2[ix] >> 8))  // are buffer1 and buffer2 different
    {
      attr = a;             // yes, output escape sequences for
      set_attribs();        // selected attributes / colors

      // output all other chars from this index on with same attribs

      jx = size;

      for(jx = size -1; jx >= ix; jx--)
      {
        if((a == (p1[jx] >> 8)) &&
           (a != (p2[jx] >> 8)))
        {
          p2[jx] = p1[jx];
          scr_emit(scr, jx);
        }
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
