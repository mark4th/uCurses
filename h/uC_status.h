// uC_status.h
// -----------------------------------------------------------------------

#ifndef UC_STATUS_H
#define UC_STATUS_H

// -----------------------------------------------------------------------

#include <stdarg.h>
#include <stdint.h>

#include <uC_screen.h>
#include <uC_window.h>

// -----------------------------------------------------------------------

API uC_window_t *uC_add_status(uC_screen_t *scr, uint16_t width,
    uint16_t xco, uint16_t yco);
API void uC_clr_status(uC_window_t *win);
API void uC_set_status(uC_window_t *win, const char *fmt, ...);

// -----------------------------------------------------------------------

#endif // UC_STATUS_H

// =======================================================================
