// uC_mouse.h
// -----------------------------------------------------------------------

#ifndef UC_MOUSE_H
#define UC_MOUSE_H

// -----------------------------------------------------------------------

#include <stdbool.h>

#include "uCurses.h"

// -----------------------------------------------------------------------

typedef struct
{
    int16_t x;          // 1-based terminal column
    int16_t y;          // 1-based terminal row
    uint8_t button;     // UC_MOUSE_LEFT/MIDDLE/RIGHT or UC_MOUSE_MOVE
    bool    pressed;    // true = press, false = release (always false for moves)
} uC_mouse_event_t;

// -----------------------------------------------------------------------

#define UC_MOUSE_LEFT      0
#define UC_MOUSE_MIDDLE    1
#define UC_MOUSE_RIGHT     2
#define UC_MOUSE_MOVE      35
#define UC_MOUSE_WHEEL_UP  64
#define UC_MOUSE_WHEEL_DN  65
#define UC_MOUSE_WHEEL_LT  66
#define UC_MOUSE_WHEEL_RT  67

// sentinel returned by uC_key() when a mouse event has been parsed
#define WIDGET_KEY_MOUSE  0x89

// -----------------------------------------------------------------------

extern uC_mouse_event_t uC_mouse_event;

// -----------------------------------------------------------------------

API void uC_mouse_enable(void);
API void uC_mouse_disable(void);

bool uC_mouse_parse(void);  // visibility hidden

// -----------------------------------------------------------------------

#endif // UC_MOUSE_H

// =======================================================================
