// tui.c   - uCurses text user interface
// -----------------------------------------------------------------------

#include "h/tui.h"
#include "h/color.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

  fp_t draw_screen;
  fp_t draw_borders;
  fp_t draw_menus;

void c_emit(uint8_t c1);

// -----------------------------------------------------------------------
// screen getter and setter primitives
// -----------------------------------------------------------------------

uint16_t scr_width_get(screen_t *scr)
{
  return scr->width;
}

uint16_t scr_height_get(screen_t *scr)
{
  return scr->height;
}

uint32_t *scr_b1_get(screen_t *scr)
{
  return scr->buffer1;
}

uint32_t *scr_b2_get(screen_t *scr)
{
  return scr->buffer2;
}

list_t *scr_win_get(screen_t *scr)
{
  return (list_t *)&scr->windows;
}

// -----------------------------------------------------------------------

void scr_width_set(screen_t *scr, uint16_t width)
{
  scr->width = width;
}

void scr_height_set(screen_t *scr, uint16_t height)
{
  scr->height = height;
}

void scr_b1_set(screen_t *scr, uint32_t *b1)
{
  scr->buffer1 = b1;
}
void scr_b2_set(screen_t *scr, uint32_t *b2)
{
  scr->buffer2 = b2;
}

void scr_win_add(screen_t *scr, window_t *win)
{
  list_append(scr->windows, (void *)win);
}

// -----------------------------------------------------------------------
// return size of screen in cells (chars)

uint32_t scr_size(screen_t *scr)
{
  return scr_width_get(scr) * scr_height_get(scr);
}

// -----------------------------------------------------------------------
// get number of bytes per screen line

uint16_t scr_bpl(screen_t *scr)
{
  return scr_width_get(scr) * sizeof(uint32_t);
}

// -----------------------------------------------------------------------
// return buffer address of screen line

uint32_t *to_scr_line(screen_t *scr, uint16_t line)
{
  return (scr_bpl(scr) * sizeof(uint32_t)) + scr_b1_get(scr);
}

// -----------------------------------------------------------------------
// window getter and setter primitives
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

uint16_t win_width_get(window_t *win)
{
  return win->width;
}

// -----------------------------------------------------------------------

uint16_t win_height_get(window_t *win)
{
  return win->height;
}

// -----------------------------------------------------------------------
// get windows x position within screen

uint16_t win_xco_get(window_t *win)
{
  return win->xco;
}

// -----------------------------------------------------------------------
// get windows y position within screen

uint16_t win_yco_get(window_t *win)
{
  return win->yco;
}

// -----------------------------------------------------------------------
// get cursor x position within window

uint16_t win_cx_get(window_t *win)
{
  return win->cx;
}

// -----------------------------------------------------------------------
// get cursor y position within window

uint16_t win_cy_get(window_t *win)
{
  return win->cy;
}

// -----------------------------------------------------------------------

win_flag_t win_flags_get(window_t *win)
{
  return win->flags;
}

// -----------------------------------------------------------------------

uint32_t *win_buff_get(window_t *win)
{
  return win->buffer;
}

// -----------------------------------------------------------------------

screen_t *win_scr_get(window_t *win)
{
  return win->screen;
}

// -----------------------------------------------------------------------
// get character to blank window with

uint16_t win_blank_get(window_t *win)
{
  return win->blank;
}

// -----------------------------------------------------------------------
// get window fg/bg/attributes etc

uint32_t win_attrs_get(window_t *win)
{
  return win->attrs;
}

// -----------------------------------------------------------------------
// get window border fg/bg/attributes etc

uint32_t win_battrs_get(window_t *win)
{
  return win->battrs;
}

// -----------------------------------------------------------------------

void win_width_set(window_t *win, uint16_t width)
{
  win->width = width;
}

// -----------------------------------------------------------------------

void win_height_set(window_t *win, uint16_t height)
{
  win->height = height;
}

// -----------------------------------------------------------------------
// set windows x position within screen

void win_xco_set(window_t *win, uint16_t xco)
{
  win->xco = xco;
}

// -----------------------------------------------------------------------
// set windows y position within screen

void win_yco_set(window_t *win, uint16_t yco)
{
  win->yco = yco;
}

// -----------------------------------------------------------------------
// set cursor x position within window

void win_cx_set(window_t *win, uint16_t cx)
{
  win->cx = cx;
}

// -----------------------------------------------------------------------
// set cursor y position within window

void win_cy_set(window_t *win, uint16_t cy)
{
  win->cy = cy;
}

// -----------------------------------------------------------------------

void win_flags_set(window_t *win, uint16_t flags)
{
  win->flags = flags;
}

// -----------------------------------------------------------------------

void win_buff_set(window_t *win, uint32_t *buffer)
{
  win->buffer = buffer;
}

// -----------------------------------------------------------------------

void win_scr_set(window_t *win, screen_t *scr)
{
  win->screen = scr;
}

// -----------------------------------------------------------------------
// set character to blank window with

void win_blank_set(window_t *win, uint16_t blank)
{
  win->blank = blank;
}

// -----------------------------------------------------------------------
// set window fg/bg/attributes etc

void win_attrs_set(window_t *win, uint32_t attr)
{
  win->attrs = attr;
}

// -----------------------------------------------------------------------
// set window border fg/bg/attributes etc

void win_battrs_set(window_t *win, uint32_t battr)
{
  win->battrs = battr;
}

// -----------------------------------------------------------------------

uint32_t win_bpl(window_t *win)
{
  return win->width * sizeof(uint32_t);
}

// -----------------------------------------------------------------------

uint32_t *to_win_line(window_t *win, uint16_t line)
{
  return win->buffer + (win_bpl(win) * line);
}

// -----------------------------------------------------------------------

uint32_t win_size(window_t *win)
{
  return win_width_get(win) * win_height_get(win);
}

// -----------------------------------------------------------------------

void win_pos_set(window_t *win, uint16_t xco, uint16_t yco)
{
  win_xco_set(win, xco);
  win_yco_set(win, yco);
}

// -----------------------------------------------------------------------

void win_at(window_t *win, uint16_t cx, uint16_t cy)
{
  win_cx_set(win, cx);
  win_cy_set(win, cy);
}

// -----------------------------------------------------------------------

void win_fg_set(window_t *win, uint8_t fg)
{
  uint16_t attr = win_attrs_get(win) & 0xfff0;
  win_attrs_set(win, attr | (fg & 0x0f));
}

// -----------------------------------------------------------------------

void win_bg_set(window_t *win, uint8_t bg)
{
  uint16_t attr = win_attrs_get(win) & 0xff0f;
  win_attrs_set(win, attr | ((bg & 0x0f) << 4 ));
}

// -----------------------------------------------------------------------

void win_attr_set(window_t *win, uint32_t attr)
{
  uint16_t a = win_attrs_get(win);
  a |= (attr << 8);
  win_attrs_set(win, a);
}

// -----------------------------------------------------------------------

void win_attr_clr(window_t *win, uint32_t attr)
{
  uint32_t a = win_attrs_get(win);
  a &= ~(attr << 8);
  win_attrs_set(win, a);
}

// -----------------------------------------------------------------------

void win_so_set(window_t *win)
{
  win_attr_set(win, STANDOUT);
}

// -----------------------------------------------------------------------

void win_so_clr(window_t *win)
{
  win_attr_clr(win, STANDOUT);
}

// -----------------------------------------------------------------------

void win_ul_set(window_t *win)
{
  win_attr_set(win, UNDERLINE);
}

// -----------------------------------------------------------------------

void win_ul_clr(window_t *win)
{
  win_attr_clr(win, UNDERLINE);
}

// -----------------------------------------------------------------------

void win_rev_set(window_t *win)
{
  win_attr_set(win, REVERSE);
}

// -----------------------------------------------------------------------

void win_rev_clr(window_t *win)
{
  win_attr_clr(win, REVERSE);
}

// -----------------------------------------------------------------------

void win_bold_set(window_t *win)
{
  win_attr_set(win, BOLD);
}

// -----------------------------------------------------------------------

void win_bold_clr(window_t *win)
{
  win_attr_clr(win, BOLD);
}

// -----------------------------------------------------------------------

void win_alt_set(window_t *win)
{
  win_attr_set(win, ACS);
}

// -----------------------------------------------------------------------

void win_alt_clr(window_t *win)
{
  win_attr_clr(win, ACS);
}

// -----------------------------------------------------------------------

void win_flag_set(window_t *win, win_flag_t flag)
{
  win_flag_t f = win_flags_get(win) | flag;
  win_flags_set(win, f);
}

// -----------------------------------------------------------------------

void win_flag_clr(window_t *win, win_flag_t flag)
{
  win_flag_t f = win_flags_get(win) & ~flag;
  win_flags_set(win, f);
}

// -----------------------------------------------------------------------

void win_borders_on(window_t *win)
{
  win_flag_set(win, WIN_BOXED);
}

// -----------------------------------------------------------------------

void win_borders_off(window_t *win)
{
  win_flag_clr(win, WIN_BOXED);
}

// -----------------------------------------------------------------------

void win_filled_on(window_t *win)
{
  win_flag_set(win, WIN_FILLED);
}

// -----------------------------------------------------------------------

void win_filled_off(window_t *win)
{
  win_flag_clr(win, WIN_FILLED);
}

// -----------------------------------------------------------------------

void win_lock(window_t *win)
{
  win_flag_set(win, WIN_LOCKED);
}

// -----------------------------------------------------------------------

void win_unlock(window_t *win)
{
  win_flag_clr(win, WIN_LOCKED);
}

// -----------------------------------------------------------------------

bool win_is_boxed(window_t *win)
{
  return (0 != win_flags_get(win) & WIN_BOXED);
}

// -----------------------------------------------------------------------

bool win_is_locked(window_t *win)
{
  return (0 != win_flags_get(win) & WIN_LOCKED);
}

// -----------------------------------------------------------------------

bool win_is_filled(window_t *win)
{
  return (0 != win_flags_get(win) & WIN_FILLED);
}

// =======================================================================
