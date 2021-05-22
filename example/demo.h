
#include "../h/uCurses.h"

#define NUM_OBJECTS 5

// -----------------------------------------------------------------------
// typedefs...

typedef struct              // a point in 3d space
{
    char x, y, z;
} xyz;

typedef struct              // a point in 2d space
{
    int x, y;
} xy;

typedef struct
{
    int counter;            // count down to next speed/shape change
    int reset;              // initial value of counter
    int *ptr;               // pointer to item to modify
    int delta;              // ammount to change item by
    int upper;              // upper limit for item
    int lower;              // lower limit for item
} modifier;


opt_t menu_address_cb(int32_t hash);
void run_demo1(screen_t *scr, window_t *win1, window_t *win2);
void do_dots(void);
void main_screen(void);
void mandel_demo(void);
void lion(void);
void raycast(void);
