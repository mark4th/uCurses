// scrdisp.c  - screen display update
// -----------------------------------------------------------------------

#include "h/tui.h"
#include "h/uCurses.h"

extern fp_t *draw_window;

extern uint32_t attr;
extern uint16_t cx;
extern uint16_t cy;

screen_t *scrn;             // current screen being drawn into

// -----------------------------------------------------------------------

void noop(void) {;}

// -----------------------------------------------------------------------

void draw_windows(screen_t *scr)
{
  window_t *win;
  node_t *n1;

  n1 = (node_t *)scr_win_get(scr);

  while(0 != n1)
  {
    win = (window_t *) n1->payload;
    (*draw_window)(win);
    n1 = n1->next;
  }
}

// -----------------------------------------------------------------------
// output cursor translation unless its already there

void if_at(uint16_t x, uint16_t y)
{
  if((x == cx) && (y != cy))
  {
     vpa(y);
     return;
  }
  if((y == cy) && (x != cx))
  {
    hpa(x);
    return;
  }
  cup(x, y);
}

// -----------------------------------------------------------------------

static uint32_t get_c(uint16_t ix, uint32_t *buffer)
{
  return(buffer[ix]);
}

// -----------------------------------------------------------------------

KEEP static void set_c(uint32_t ix, uint32_t *buffer, uint32_t c)
{
  buffer[ix] = c;
}

// -----------------------------------------------------------------------

static uint32_t *get_b1(void)
{
  return(scrn->buffer1);
}

// -----------------------------------------------------------------------

static uint32_t *get_b2(void)
{
  return(scrn->buffer2);
}

// -----------------------------------------------------------------------

uint32_t get_c1(uint16_t ix)
{
  uint32_t *buffer = get_b1();
  return get_c(ix, buffer);
}

// -----------------------------------------------------------------------

uint32_t get_c2(uint16_t ix)
{
  uint32_t *buffer = get_b2();
  return get_c(ix, buffer);
}

// -----------------------------------------------------------------------

void set_c1(uint16_t ix, uint16_t c)
{
  uint32_t *buffer = get_b1();
  set_c(ix, buffer, c);
}

// -----------------------------------------------------------------------

void set_c2(uint16_t ix, uint16_t c)
{
  uint32_t *buffer = get_b2();
  set_c(ix, buffer, c);
}

// -----------------------------------------------------------------------

KEEP static bool if_modified(uint16_t index)
{
  uint32_t c1 = get_c1(index);
  uint32_t c2 = get_c2(index);

  return (c1 != c2);
}

// -----------------------------------------------------------------------

KEEP static void to_attribs(uint16_t attribs)
{
  attr = attribs;
  set_attribs();
}

// -----------------------------------------------------------------------

void scr_emit(uint16_t index)
{
  uint32_t c1 = get_c1(index);
  uint16_t width = scr_width_get(scrn);
  uint16_t y = index / width;
  uint16_t x = (width - y);
  if_at(x, y);

  set_c2(index, c1);
  // of c1 is not an ascii char its an index into a
  // utf8 table
  c_emit(c1 & 0xff);
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

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

void xxxxx_scr_emit(screen_t *scr, uint32_t ix)
{
  uint32_t c;
  uint16_t width;
  uint16_t x, y;
  uint16_t cx = 0, cy = 0;
  uint32_t *p1, *p2;

  p1 = scr_b1_get(scr);
  p2 = scr_b2_get(scr);

  width = scr_width_get(scr);
  c = p1[ix];
  p2[ix] = c;

  y = ix / width;
  x = ix - y;  // % width;

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

  send_str = (fp_t *)&noop;
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
//          scr_emit(scr, jx);
        }
      }
    }
  }
  attr = attr_save;
  set_attribs();
  send_str = (fp_t *)&_send_str;       // initialize function pointers
  _send_str(0);
}

// -======================================================================
