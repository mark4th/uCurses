// windisp.c  - window display functions
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/uCurses.h"
#include "h/tui.h"

// -----------------------------------------------------------------------
// returns 00xxyycc where xx = attribs, yy = colors and cc = char

static uint32_t make_char(window_t *win, uint16_t c)
{
  return (win_attrs_get(win) << 16) | c;
}

// -----------------------------------------------------------------------

static uint32_t make_blank(window_t *win)
{
  return (win_attrs_get(win) << 16) |  win_blank_get(win);
}

// -----------------------------------------------------------------------

static void erase_line(window_t *win, uint16_t line)
{
  uint32_t blank = make_blank(win);
  uint16_t len = win_width_get(win);
  uint32_t *p = to_win_line(win, line);

  while(0 != len--)
  {
    *p++ = blank;
  }
}

// -----------------------------------------------------------------------

static void copy_line(window_t *win, uint16_t src, uint16_t dst)
{
  uint32_t *sp = to_win_line(win, src);
  uint32_t *dp = to_win_line(win, dst);
  uint16_t len = win_width_get(win);
  uint32_t blank = make_blank(win);

  while(0 != len--)
  {
    *dp = *sp++;
    *sp++ = blank;
  }
}

// -----------------------------------------------------------------------

static void scroll_up(window_t *win)
{
  uint16_t i = 0;
  uint16_t height = win_height_get(win) - 1;

  while(i++ != height)
  {
    copy_line(win, i + 1 , i);
  }
}

// -----------------------------------------------------------------------

static void if_scroll_up(window_t *win)
{
  if(~win_is_locked(win))
  {
    scroll_up(win);
  }
}

// -----------------------------------------------------------------------

static void scroll_dn(window_t *win)
{
  uint16_t i = win_height_get(win) - 1;

  while(0 != i--)
  {
    copy_line(win, i, i + 1);
  }
}

// -----------------------------------------------------------------------

KEEP static void if_scroll_dn(window_t *win)
{
  if(~win_is_locked(win))
  {
    scroll_dn(win);
  }
}

// -----------------------------------------------------------------------

static void _pan_left(window_t *win, uint16_t line)
{
  uint32_t blank = make_blank(win);
  uint16_t width = win_width_get(win) - 1;
  uint32_t *p = to_win_line(win, line);

  memcpy(p, p + 1, width);
  p[width + 1] = blank;
}

// -----------------------------------------------------------------------

static void pan_left(window_t *win)
{
  uint16_t height = win_height_get(win);

  for(;0 != height; height--)
  {
    _pan_left(win, height);
  }
}

// -----------------------------------------------------------------------

KEEP static void if_pan_left(window_t *win)
{
  if(~win_is_locked(win))
  {
    pan_left(win);
  }
}

// -----------------------------------------------------------------------

static void _pan_right(window_t *win, uint16_t line)
{
  uint32_t blank = make_blank(win);
  uint16_t width = win_width_get(win) - 1;
  uint32_t *p = to_win_line(win, line);

  while(0xffff != width--)
  {
    p[width] = p[width - 1];
  }
  p[0] = blank;
}

// -----------------------------------------------------------------------

static void pan_right(window_t *win)
{
  uint16_t height = win_height_get(win);

  for(;0 != height; height--)
  {
    _pan_right(win, height);
  }
}

// -----------------------------------------------------------------------

KEEP static void if_pan_right(window_t *win)
{
  if(~win_is_locked(win))
  {
    pan_right(win);
  }
}

// -----------------------------------------------------------------------

KEEP static void cursor_up(window_t *win)
{
  uint16_t cy = win_cy_get(win);

  if(0 != cy)
  {
    win_cy_set(win, cy - 1);
  }
}

// -----------------------------------------------------------------------

static void cursor_dn(window_t *win)
{
  uint16_t cy = win_cy_get(win);
  uint16_t height = win_height_get(win);

  (cy == height)
    ? if_scroll_up(win)
    : win_cy_set(win, cy + 1);
}

// -----------------------------------------------------------------------

static inline void _cursor_lt(window_t *win)
{
  uint16_t width;
  uint16_t cy = win_width_get(win);

  if(0 != cy)
  {
    width = win_width_get(win);
    win_cx_set(win, width);
    win_cy_set(win, cy - 1);
  }
}

// -----------------------------------------------------------------------

KEEP static void cursor_lt(window_t *win)
{
  uint16_t cx = win_cx_get(win);

  (0 != cx)
    ? win_cx_set(win, cx)
    : _cursor_lt(win);
}

// -----------------------------------------------------------------------

static inline void _cursor_rt(window_t *win)
{
  uint16_t height = win_height_get(win);
  uint16_t cy = win_cy_get(win);

  if(cy != height)
  {
    win_cy_set(win, cy + 1);
    win_cx_set(win, 0);
  }
  else if(~win_is_locked(win))
  {
    scroll_up(win);
    win_cx_set(win, 0);
  }
}

// -----------------------------------------------------------------------

static void cursor_rt(window_t *win)
{
  uint16_t cx = win_cx_get(win);
  uint16_t width = win_width_get(win);

  (cx != width)
    ? win_cx_set(win, cx + 1)
    : _cursor_rt(win);
}

// -----------------------------------------------------------------------

static void win_cr(window_t *win)
{
  win_cx_set(win, 0);
  cursor_dn(win);
}

// -----------------------------------------------------------------------

static void _win_emit(window_t *win, uint16_t c)
{
  uint16_t cy = win_cy_get(win);
  uint16_t cx = win_cx_get(win);
  uint32_t *line = to_win_line(win, cy);

  line[cx] = make_char(win, c);
}

// -----------------------------------------------------------------------

void win_emit(window_t *win, uint16_t c)
{
  switch(c)
  {
    case 0x0d: win_cr(win); break;
    case 0x0a: break;

    default:
      _win_emit(win, c);
      cursor_rt(win);
  }
}

// -----------------------------------------------------------------------

void win_type(window_t *win, uint16_t *msg)
{
  while(*msg)
  {
    win_emit(win, *msg);
    msg++;
  }
}

// -----------------------------------------------------------------------

void clear_win(window_t *win)
{
  uint16_t line = win_height_get(win);

  while(line)
  {
    erase_line(win, --line);
  }
}

// -----------------------------------------------------------------------

void set_filled(window_t *win)
{
  win_blank_set(win, 'a');
  win_filled_on(win);
}

// -----------------------------------------------------------------------

void clr_filled(window_t *win)
{
  win_blank_set(win, ' ');
  win_filled_off(win);
}

// -----------------------------------------------------------------------

static void draw_row(window_t *win, uint16_t row)
{
  screen_t *scr = win_scr_get(win);

  uint32_t *src = to_win_line(win, row);
  uint16_t yco = win_yco_get(win);
  uint16_t xco = win_xco_get(win);
  uint16_t bpl = win_bpl(win);
  uint32_t *dst = to_scr_line(scr, yco);

  memcpy(&dst[xco], src, bpl);
}

// -----------------------------------------------------------------------

KEEP void draw_win(window_t *win)
{
  screen_t *scr = win_scr_get(win);
  uint16_t height = win_height_get(win);
  uint16_t scrh = scr_height_get(scr);
  uint16_t yco = win_yco_get(win);

  if((height + yco) >= scrh)
  {
    height = scrh - (height + yco);
  }

  while(height)
  {
    draw_row(win, height--);
  }
}

// =======================================================================
