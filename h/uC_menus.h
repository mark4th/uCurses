// menus.h
// -----------------------------------------------------------------------

#ifndef MENUS_H
#define MENUS_H

#include "uCurses.h"
#include "uC_screen.h"

// -----------------------------------------------------------------------
// menu option vectors

typedef void (*menu_fp_t)(void);
typedef menu_fp_t (*fp_finder_t)(int32_t hash);

// -----------------------------------------------------------------------
// default attribs, not used by json parser, there are no defaults there

#define ATTRS_NORMAL   (0x0004030000000080)
#define ATTRS_SELECTED (0x0001060000000080)
#define ATTRS_DISABLED (0x00080400000000c2)

#define MAX_STATUS (39)

// -----------------------------------------------------------------------
// an anonymous enum with various constants

// these will probably be split up later

enum
{
    MENU_DISABLED  = 1,
    MAX_MENU_ITEMS = 10
};

// -----------------------------------------------------------------------

enum
{
   MENU_UP    =  1,
   MENU_LEFT  =  1,
   MENU_DOWN  = -1,
   MENU_RIGHT = -1
};

// -----------------------------------------------------------------------

typedef struct
{
    char *name;
    int16_t flags;          // disable flags etc
    menu_fp_t fp;           // function to execute
    int16_t shortcut;       // keyboard shortcut
} menu_item_t;

// -----------------------------------------------------------------------

typedef struct
{
    char *name;             // menu bar name for this pulldown menu
    int16_t width;          // width of widest item in pulldown menu
    int16_t flags;          // masks for enabled/disabled etc
    int16_t which;          // current selected item
    int16_t xco;            // x coordinate of menu window
    int16_t count;
    // not a linked list of sub items. max 10
    menu_item_t *items[MAX_MENU_ITEMS];
    void *window;           // this is a window_t honest!
    win_attr_grp_t attr_grp;
} pulldown_t;

// -----------------------------------------------------------------------

typedef struct
{
    void *window;           // fwd ref to window_t * grrr (c sucks)
    int16_t xco;            // x coordinate of next pulldown
    int16_t active;         // 0 = not active
    int16_t which;          // which pulldown item is active
    int16_t count;          // number of pulldowns defined

    pulldown_t *items[MAX_MENU_ITEMS];

    win_attr_grp_t attr_grp;
} menu_bar_t;

// -----------------------------------------------------------------------
// visibility hidden

void bar_populdate_pd(pulldown_t *pd);
int32_t new_menu_item(screen_t *scr, char *name, menu_fp_t fp,
    int16_t shortcut);
void menu_up(void);
void menu_down(void);
void menu_left(void);
void menu_right(void);
void pd_disable(screen_t *scr, char *name);
void pd_enable(screen_t *scr, char *name);
int32_t bar_create_pd_win(screen_t *scr, pulldown_t *pd);
int32_t new_pulldown(screen_t *scr, char *name);
void free_status(void);

// -----------------------------------------------------------------------

API void uC_bar_draw_text(screen_t *scr);
API void uC_bar_close(screen_t *scr);
API int32_t uC_bar_open(screen_t *scr);
API void uC_menu_init(void);

API void uC_bar_clr_status(void);
API void uC_alloc_status(void);
API void uC_bar_set_status(char *s);
API void uC_bar_draw_status(menu_bar_t *bar);

// -----------------------------------------------------------------------

#endif // MENUS_H

// =======================================================================
