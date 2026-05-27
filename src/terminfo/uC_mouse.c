// uC_mouse.c
// -----------------------------------------------------------------------

#include <sys/types.h>
#include <unistd.h>

#include "uCurses.h"
#include "uC_mouse.h"
#include "uC_terminfo.h"

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;

API uC_mouse_event_t uC_mouse_event;

// -----------------------------------------------------------------------

static const char mouse_enable[]  = "\033[?1000h\033[?1006h\033[?1003h";
static const char mouse_disable[] = "\033[?1003l\033[?1006l\033[?1000l";

API void uC_mouse_enable(void)
{
    ssize_t w = write(1, mouse_enable,  sizeof(mouse_enable)  - 1);
    (void)w;
}

API void uC_mouse_disable(void)
{
    ssize_t w = write(1, mouse_disable, sizeof(mouse_disable) - 1);
    (void)w;
}

// -----------------------------------------------------------------------
// detect and parse an SGR mouse event: ESC [ < btn ; x ; y M|m
// on success fills uC_mouse_event, stuffs WIDGET_KEY_MOUSE, returns true

bool uC_mouse_parse(void)
{
    uint8_t *buf = ti_vars->keybuff;
    int16_t  num = ti_vars->num_k;
    int16_t  i   = 3;
    int      btn = 0;
    int      x   = 0;
    int      y   = 0;

    if (num < 8)        return false;
    if (buf[0] != 0x1b) return false;
    if (buf[1] != '[')  return false;
    if (buf[2] != '<')  return false;

    while (i < num && buf[i] != ';') btn = btn * 10 + (buf[i++] - '0');
    if (i >= num || buf[i] != ';')   return false;
    i++;

    while (i < num && buf[i] != ';') x = x * 10 + (buf[i++] - '0');
    if (i >= num || buf[i] != ';')   return false;
    i++;

    while (i < num && buf[i] != 'M' && buf[i] != 'm')
    {
        y = y * 10 + (buf[i++] - '0');
    }
    if (i >= num || (buf[i] != 'M' && buf[i] != 'm')) return false;

    uC_mouse_event.button  = (uint8_t)btn;
    uC_mouse_event.x       = (int16_t)x;
    uC_mouse_event.y       = (int16_t)y;
    uC_mouse_event.pressed = (buf[i] == 'M');

    ti_vars->keybuff[0] = WIDGET_KEY_MOUSE;
    ti_vars->num_k      = 1;

    return true;
}

// =======================================================================
