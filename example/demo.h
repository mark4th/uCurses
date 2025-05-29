
#include "uCurses.h"
#include "uC_menus.h"
#include "uC_utf8.h"
#include "uC_utils.h"
#include "uC_keys.h"
#include "uC_win_printf.h"
#include "uC_status.h"
#include "uC_terminfo.h"
#include "uC_attribs.h"
#include "uC_switch.h"
#include "uC_json.h"
#include "uC_braille.h"

#define NUM_OBJECTS 5

// -----------------------------------------------------------------------

extern char status[33];
extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------
// typedefs...

typedef struct              // a point in 3d space
{
    char x, y, z;
} xyz;

// -----------------------------------------------------------------------

typedef struct              // a point in 2d space
{
    int x, y;
} xy;

// -----------------------------------------------------------------------

typedef struct
{
    int counter;            // count down to next speed/shape change
    int reset;              // initial value of counter
    int *ptr;               // pointer to item to modify
    int delta;              // ammount to change item by
    int upper;              // upper limit for item
    int lower;              // lower limit for item
} modifier;

// -----------------------------------------------------------------------

opt_t menu_address_cb(int32_t hash);

// =======================================================================
