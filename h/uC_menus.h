// menus.h
// -----------------------------------------------------------------------

#ifndef UC_MENUS_H
#define UC_MENUS_H

#include "uCurses.h"
#include "uC_screen.h"

// -----------------------------------------------------------------------
// menu option vectors

typedef void (*menu_fp_t)(void);
typedef menu_fp_t (*fp_finder_t)(int32_t hash);

// -----------------------------------------------------------------------
// an anonymous enum with various constants

// these will probably be split up later

enum __attribute__((__packed__))
{
    MENU_DISABLED  = 1,
    MAX_MENU_ITEMS = 10
};

// -----------------------------------------------------------------------

enum __attribute__((__packed__))
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
    void *window;           // this is a uC_window_t honest!
    uC_attribs_t attrs;
    uC_attribs_t selected_attrs;
    uC_attribs_t disabled_attrs;
} pulldown_t;

// -----------------------------------------------------------------------

typedef struct
{
    void *window;           // fwd ref to uC_window_t * grrr (c sucks)
    int16_t xco;            // x coordinate of next pulldown
    int16_t active;         // 0 = not active
    int16_t which;          // which pulldown item is active
    int16_t count;          // number of pulldowns defined

    pulldown_t *items[MAX_MENU_ITEMS];

    uC_attribs_t attrs;
    uC_attribs_t selected_attrs;
    uC_attribs_t disabled_attrs;
} menu_bar_t;

// -----------------------------------------------------------------------
// visibility hidden

void bar_populdate_pd(pulldown_t *pd);
int32_t new_menu_item(uC_screen_t *scr, char *name, menu_fp_t fp,
    int16_t shortcut);
void menu_up(void);
void menu_down(void);
void menu_left(void);
void menu_right(void);
void pd_disable(uC_screen_t *scr, char *name);
void pd_enable(uC_screen_t *scr, char *name);
int32_t bar_create_pd_win(uC_screen_t *scr, pulldown_t *pd);
int32_t new_pulldown(uC_screen_t *scr, char *name);

// -----------------------------------------------------------------------

API void uC_bar_draw_text(uC_screen_t *scr);
API void uC_bar_close(uC_screen_t *scr);
API int32_t uC_bar_open(uC_screen_t *scr);
API void uC_menu_init(void);

// -----------------------------------------------------------------------

#endif // UC_MENUS_H

// =======================================================================
