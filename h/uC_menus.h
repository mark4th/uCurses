// uC_menus.h
// -----------------------------------------------------------------------

#ifndef UC_MENUS_H
#define UC_MENUS_H

// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_screen.h"

// -----------------------------------------------------------------------
// an anonymous enum with various constants

// these will probably be split up later

enum
{
    uC_MENU_DISABLED  = 1,
    uC_MAX_MENU_ITEMS = 10
} __attribute__((__packed__)) ;

// -----------------------------------------------------------------------

enum
{
   MENU_UP    =  1,
   MENU_LEFT  =  1,
   MENU_DOWN  = -1,
   MENU_RIGHT = -1
} __attribute__((__packed__)) ;

// -----------------------------------------------------------------------

typedef struct
{
    const char *name;
    int16_t flags;          // disable flags etc
    menu_fp_t fp;           // function to execute
    uC_shortcut_t shortcut; // keyboard shortcut
    uC_screen_t *screen;
    struct pulldown_s *parent;
    bool shortcut_registered;
} menu_item_t;

// -----------------------------------------------------------------------

typedef struct pulldown_s
{
    const char *name;       // menu bar name for this pulldown menu
    int16_t width;          // width of widest item in pulldown menu
    int16_t flags;          // masks for enabled/disabled etc
    int16_t which;          // current selected item
    int16_t xco;            // x coordinate of menu window
    int16_t count;
    // not a linked list of sub items. max 10
    menu_item_t *items[uC_MAX_MENU_ITEMS];
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

    pulldown_t *items[uC_MAX_MENU_ITEMS];

    uC_attribs_t attrs;
    uC_attribs_t selected_attrs;
    uC_attribs_t disabled_attrs;
} menu_bar_t;

// -----------------------------------------------------------------------
// visibility hidden

void draw_pd(pulldown_t *pd);
int32_t bar_create_pd_win(uC_screen_t *scr, pulldown_t *pd);
bool menu_key(uC_screen_t *scr, uint8_t key, uint8_t *out);
void uC_menu_deinit_keys(void);
bool uC_menu_shortcut_display(uC_shortcut_t shortcut, char *dst,
    uint16_t dst_len);
uint16_t uC_menu_shortcut_width(uC_shortcut_t shortcut);
bool uC_menu_shortcut_matches(uC_shortcut_t shortcut, uint8_t key);
bool menu_item_shortcut_register(pulldown_t *pd, menu_item_t *item);
void menu_item_shortcut_remove(menu_item_t *item);
void menu_pd_shortcuts_register(pulldown_t *pd);
void menu_pd_shortcuts_remove(pulldown_t *pd);
void menu_normalize_selection(uC_screen_t *scr);

// -----------------------------------------------------------------------

API int32_t uC_menu_bar_open(uC_screen_t *scr);
API void uC_menu_bar_close(uC_screen_t *scr);
API int32_t uC_menu_new_pd(uC_screen_t *scr, const char *name);
API int32_t uC_menu_new_item(uC_screen_t *scr, const char *name, menu_fp_t fp,
    uC_shortcut_t shortcut);
API void uC_menu_pd_disable(uC_screen_t *scr, const char *name);
API void uC_menu_pd_enable(uC_screen_t *scr, const char *name);
API void uC_menu_item_disable(uC_screen_t *scr, const char *pd_name,
    const char *item_name);
API void uC_menu_item_enable(uC_screen_t *scr, const char *pd_name,
    const char *item_name);
API void uC_menu_init_keys(void);
API bool uC_menu_is_active(uC_screen_t *scr);
API void uC_menu_open(uC_screen_t *scr);
API void uC_menu_close(uC_screen_t *scr);
API uint8_t uC_menu_run(uC_screen_t *scr);
API void uC_menu_cursor_up(uC_screen_t *scr);
API void uC_menu_cursor_down(uC_screen_t *scr);
API void uC_menu_cursor_left(uC_screen_t *scr);
API void uC_menu_cursor_right(uC_screen_t *scr);
API void uC_menu_select(uC_screen_t *scr);

// -----------------------------------------------------------------------

#endif // UC_MENUS_H

// =======================================================================
