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

extern uint16_t attr;

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
  screen_t *scr = win_scr_get(win);

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

  size = scr_size(scr);     // allocate 2 buffers at once
  p = (uint32_t *)malloc(size * CELL * 2);

  if(NULL == p)
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

// =======================================================================
