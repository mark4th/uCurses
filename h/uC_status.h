// uC_status.h
// -----------------------------------------------------------------------

#include <stdint.h>

// -----------------------------------------------------------------------

API uC_window_t *uC_add_status(uC_screen_t *scr, uint16_t w,
    uint16_t x, uint16_t y);
API void uC_clr_status(uC_window_t *win);
API void uC_set_status(uC_window_t *win, char *s);

// =======================================================================
