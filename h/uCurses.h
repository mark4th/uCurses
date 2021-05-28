// uCurses.h
// -----------------------------------------------------------------------

#ifndef UCURSES_H
#define UCURSES_H

// -----------------------------------------------------------------------

#include <aio.h>
#include <inttypes.h>

#include "list.h"

// -----------------------------------------------------------------------

#define INLINE inline __attribute__((always_inline))
// #define INLINE

// -----------------------------------------------------------------------

#define UCURSES_NAN 0x7fff // not a number (honest!)

#define MAX_PARAM 9
extern int64_t params[MAX_PARAM];

// -----------------------------------------------------------------------
// utf-8 codepoint for 'blank' char used in screen backdrop windows

#define SOLID 0x2592
#define SHADOW 0x2591

#define MENU_DISABLED 1
#define MAX_MENU_ITEMS 10
#define DEADCODE 0xdeadc0de
#define FAR 0x7fff

// allows me to sset the 8 byte uint8_t array with one uint64_t write
#define attrs8 (uint64_t *)&attrs[0]

// -----------------------------------------------------------------------

typedef struct
{
    union
    {
        uint8_t bytes[8];
        uint64_t chunk;
    };
} attribs_t;

// -----------------------------------------------------------------------

typedef void (*opt_t)(void);     // re_switch vectors
typedef void (*menu_fp_t)(void); // menu option vectors
typedef void (*fp_t)(void *x);

typedef menu_fp_t (*fp_finder_t)(int32_t hash);

// -----------------------------------------------------------------------

typedef void key_handler_t(void);

typedef enum
{
    K_ENT,
    K_CUU1,
    K_CUD1,
    K_CUB1,
    K_CUF1,
    K_BS,
    K_BS2,
    K_DCH1,
    K_ICH1,
    K_HOME,
    K_END,
    K_KNP,
    K_KPP,
    K_F1,
    K_F2,
    K_F3,
    K_F4,
    K_F5,
    K_F6,
    K_F7,
    K_F8,
    K_F9,
    K_F10,
    K_F11,
    K_f12
} key_index_t;

// -----------------------------------------------------------------------
// c switch statements are FUGLY

typedef struct
{
    int32_t option;
    opt_t vector;
} switch_t;

// -----------------------------------------------------------------------

typedef enum
{
    BDR_TOP_LEFT,
    BDR_TOP_RIGHT,
    BDR_BOTTOM_LEFT,
    BDR_BOTTOM_RIGHT,
    BDR_HORIZONTAL,
    BDR_VERTICAL,
    BDR_LEFT_T,
    BDR_RIGHT_T,
    BDR_TOP_T,
    BDR_BOTTOM_T,
    BDR_CROSS,
} border_t;

// -----------------------------------------------------------------------

enum
{
    BDR_SINGLE,
    BDR_DOUBLE,
    BDR_CURVED
};

// -----------------------------------------------------------------------
// window flags

typedef enum
{
    WIN_BOXED = 1,  // has a border
    WIN_LOCKED = 2, // scroll locked
    WIN_FILLED = 4  // backfilled with SOLID character
} win_flags_t;

// --------------------------------------------------------------------------

typedef struct
{
    int8_t len;
    int8_t str[4];
    int width;
} utf8_encode_t;

// -----------------------------------------------------------------------
// structure of each cell of a window / screen

typedef struct
{
    attribs_t attrs; // bold, underline, gray scale, rgb
    int32_t code;    // utf-8 codepoint
} cell_t;

// -----------------------------------------------------------------------

typedef struct uCurses
{
    attribs_t attrs;
    union
    {
        attribs_t selected;
        attribs_t old_attrs;
    };
    union
    {
        attribs_t disabled;
        attribs_t bdr_attrs;
    };
} attrib_group_t;

// -----------------------------------------------------------------------

typedef struct
{
    char *name;
    int16_t flags;    // disable flags etc
    menu_fp_t fp;     // function to execute
    int16_t shortcut; // keyboard shortcut
} menu_item_t;

// -----------------------------------------------------------------------

typedef struct
{
    char *name;    // menu bar name for this pulldown menu
    int16_t width; // width of widest item in pulldown menu
    int16_t flags; // masks for enabled/disabled etc
    int16_t which; // current selected item
    int16_t xco;   // x coordinate of menu window
    int16_t count;
    // not a linked list of sub items. max 10
    menu_item_t *items[MAX_MENU_ITEMS];
    void *window; // this is a window_t honest!
    attrib_group_t attr_group;
} pulldown_t;

// -----------------------------------------------------------------------

typedef struct
{
    void *window;   // fwd ref to window_t * grrr (c sucks)
    int16_t xco;    // x coordinate of next pulldown
    int16_t active; // 0 = not active
    int16_t which;  // which pulldown item is active
    int16_t count;  // number of pulldowns defined

    pulldown_t *items[MAX_MENU_ITEMS];

    attrib_group_t attr_group;
} menu_bar_t;

// -----------------------------------------------------------------------

typedef struct
{
    node_t *links;
    cell_t *buffer;
    void *screen;
    win_flags_t flags;
    int32_t blank; // window fill character for backdrop windows
    int16_t width; // window dimensions
    int16_t height;
    int16_t xco; // window x/y coordinat within screen
    int16_t yco;
    int16_t cx; // cursor position within window
    int16_t cy;
    int16_t bdr_type;

    attrib_group_t attr_group;
} window_t;

// -----------------------------------------------------------------------

typedef struct
{
    list_t windows; // linked list of windows
    menu_bar_t *menu_bar;
    cell_t *buffer1; // screen buffer 1 and 2
    cell_t *buffer2;
    window_t *backdrop;
    int16_t width; // screen dimensions
    int16_t height;
    int16_t cx; // cursor corrdinates within screen
    int16_t cy;
} screen_t;

extern screen_t *active_screen;

// -----------------------------------------------------------------------

typedef enum
{
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    GRAY,
    LT_RED,
    LT_GREEN,
    YELLOW,
    LT_BLUE,
    LT_MAGENTA,
    LT_CYAN,
    LT_WHITE
} ti_color_t;

// -----------------------------------------------------------------------

typedef enum
{
    TI_UNDERLINE,
    TI_REVERSE,
    TI_BOLD,
    TI_FG_RGB,
    TI_BG_RGB,
    TI_FG_GRAY,
    TI_BG_GRAY,

    UNDERLINE = (1 << TI_UNDERLINE),
    REVERSE = (1 << TI_REVERSE),
    BOLD = (1 << TI_BOLD),

    FG_RGB = (1 << TI_FG_RGB),
    BG_RGB = (1 << TI_BG_RGB),
    FG_GRAY = (1 << TI_FG_GRAY),
    BG_GRAY = (1 << TI_BG_GRAY),
} ti_attrib_t;

// -----------------------------------------------------------------------
// indicies into attributes array

typedef enum
{
    ATTR,
    FG = 1,
    FG_R = 1,
    BG = 2,
    BG_R = 2,
    FG_G,
    BG_G,
    FG_B,
    BG_B,
} attr_index_t;

// -----------------------------------------------------------------------

void attr_set_attr(attribs_t *attribs, int8_t attr);
void attr_clr_attr(attribs_t *attribs, ti_attrib_t attr);
void attr_set_bytes(attribs_t *attribs, attr_index_t which,
                    ti_color_t color);

#define console_set_attr(attr) attr_set_attr(&attrs, attr)
#define console_clr_attr(attr) attr_clr_attr(&attrs, attr)
#define console_set_bytes(which, color) attr_set_byes(&attrs, which, color)

#define console_set_ul console_set_attr(UNDERLINE)
#define console_clr_ul console_clr_attr(UNDERLINE)
#define console_set_rev console_set_attr(REVERSE)
#define console_clr_rev console_clr_attr(REVERSE)
#define console_set_bold console_set_attr(BOLD)
#define console_clr_bold console_clr_attr(BOLD)

#define console_set_fg(color)                                             \
    attr_set_bytes(&attrs, FG, color);                                    \
    attr_clr_attr(&attrs, FG_RGB | FG_GRAY)

#define console_set_bg(color)                                             \
    attr_set_bytes(&attrs, BG, color);                                    \
    attr_clr_attr(&attrs, BG_RGB | BG_GRAY)

#define console_set_gray_fg(color)                                        \
    attr_set_bytes(&attrs, FG, color);                                    \
    attr_set_attr(FG_GRAY)

#define console_set_gray_bg(color)                                        \
    attr_set_bytes(&attrs, BG, color);                                    \
    attr_set_attr(BG_GRAY)

#define console_set_rgb_fg(r, g, b)                                       \
    attr_set_bytes(&attrs, FG_R, r);                                      \
    attr_set_bytes(&attrs, FG_G, g);                                      \
    attr_set_bytes(&attrs, FG_B, b);                                      \
    attr_set_attr(FG_RGB)

#define console_set_rgb_bg(r, g, b)                                       \
    attr_set_bytes(&attrs, BG_R, r);                                      \
    attr_set_bytes(&attrs, BG_G, g);                                      \
    attr_set_bytes(&attrs, BG_B, b);                                      \
    attr_set_attr(BG_RGB)

// -----------------------------------------------------------------------

extern attribs_t attrs;

#define win_set_attr(attribs, attr)                                       \
    attr_set_attr(&win->attr_group.attrs, attr)

#define win_clr_attr(win, attr) attr_clr_attr(&win->attr_group.attrs, attr)

#define win_set_bdr_attr(win, attr)                                       \
    attr_set_attr(&win->attr_group.bdr_attrs, attr)

#define win_clr_bdr_attrs(win, attr)                                      \
    attr_clr_attr(&win->attr_group.bdr_attrs, attr)

#define win_set_fg(win, color)                                            \
    attr_set_bytes(&win->attr_group.attrs, FG, color)

#define win_set_bg(win, color)                                            \
    attr_set_bytes(&win->attr_group.attrs, BG, color)

#define win_set_gray_fg(win, color)                                       \
    attr_set_bytes(&win->attr_group.attrs, FG, color);                    \
    attr_set_attr(&win->attr_group.attrs, FG_GRAY)

#define win_set_gray_bg(win, color)                                       \
    attr_set_bytes(&win->attr_group.attrs, BG, color);                    \
    attr_set_attr(&win->attr_group.attrs, BG_GRAY)

#define win_set_rgb_fg(win, r, g, b)                                      \
    attr_set_bytes(&win->attr_group.attrs, FG_R, r);                      \
    attr_set_bytes(&win->attr_group.attrs, FG_G, g);                      \
    attr_set_bytes(&win->attr_group.attrs, FG_B, b);                      \
    attr_set_attr(&win->attr_group.attrs, FG_RGB)

#define win_set_rgb_bg(win, r, g, b)                                      \
    attr_set_bytes(&win->attr_group.attrs, BG_R, r);                      \
    attr_set_bytes(&win->attr_group.attrs, BG_G, g);                      \
    attr_set_bytes(&win->attr_group.attrs, BG_B, b);                      \
    attr_set_attr(&win->attr_group.attrs, BG_RGB)

#define win_set_bold(win) attr_set_attr(&win->attr_group.attrs, BOLD)

#define win_clr_bold(win) attr_clr_attr(&win->attr_group.attrs, BOLD)

#define win_set_rev(win) attr_set_attr(&win->attr_group.attrs, REVERSE)

#define win_clr_rev(win) attr_clr_attr(&win->attr_group.attrs, REVERSE)

#define win_set_ul(win) attr_set_attr(&win->attr_group.attrs, UNDERLINE)

#define win_clr_ul(win) attr_clr_attr(&win->attr_group.attrs, UNDERLINE)

#define win_set_bdr_fg(win, color)                                        \
    attr_set_bytes(&win->attr_group.bdr_attrs, FG, color)

#define win_set_bdr_bg(win, color)                                        \
    attr_set_bytes(&win->attr_group.bdr_attrs, BG, color)

#define win_set_bdr_gray_fg(win, color)                                   \
    attr_set_bytes(&win->attr_group.bdr_attrs, FG, color);                \
    attr_set_attr(&win->attr_group.bdr_attrs, FG_GRAY)

#define win_set_bdr_gray_bg(win, color)                                   \
    attr_set_bytes(&win->attr_group.bdr_attrs, BG, color);                \
    attr_set_attr(&win->attr_group.bdr_attrs, BG_GRAY)

#define win_set_bdr_rgb_fg(win, r, g, b)                                  \
    attr_set_bytes(&win->attr_group.bdr_attrs, FG_R, r);                  \
    attr_set_bytes(&win->attr_group.bdr_attrs, FG_G, g);                  \
    attr_set_bytes(&win->attr_group.bdr_attrs, FG_B, b);                  \
    attr_set_attr(&win->attr_group.bdr_attrs, FG_RGB)

#define win_set_bdr_rgb_bg(win, r, g, b)                                  \
    attr_set_bytes(&win->attr_group.bdr_attrs, BG_R, r);                  \
    attr_set_bytes(&win->attr_group.bdr_attrs, BG_G, g);                  \
    attr_set_bytes(&win->attr_group.bdr_attrs, BG_B, b);                  \
    attr_set_attr(&win->attr_group.bdr_attrs, BG_RGB)

// -----------------------------------------------------------------------

#define MAX_LINE_LEN 128
#define TOKEN_LEN 34 // 32 chars plus two quote chars

// -----------------------------------------------------------------------

typedef enum
{
    STATE_NONE,
    STATE_L_BRACE,
    STATE_KEY,
    STATE_VALUE,
    STATE_R_BRACE,
    STATE_DONE,
    NUM_STATES
} json_state_t;

// -----------------------------------------------------------------------

typedef enum
{
    STRUCT_SCREEN = 1,
    STRUCT_WINDOW,
    STRUCT_BACKDROP,
    STRUCT_MENU_BAR,
    STRUCT_PULLDOWN,
    STRUCT_MENU_ITEM,
    STRUCT_ATTRIBS,   // normal attribs
    STRUCT_B_ATTRIBS, // border attribs
    STRUCT_S_ATTRIBS, // selected attribs
    STRUCT_D_ATTRIBS, // disabled attribs
    STRUCT_FLAGS,
    STRUCT_RGB_FG, // 3 bytes
    STRUCT_RGB_BG, // 3 bytes
    STRUCT_WINDOWS,
    STRUCT_PULLDOWNS,
    STRUCT_MENU_ITEMS,
    KEY_FG,
    KEY_BG,
    KEY_GRAY_BG,
    KEY_GRAY_FG,
    KEY_RED,
    KEY_GREEN,
    KEY_BLUE,
    KEY_XCO,
    KEY_YCO,
    KEY_WIDTH,
    KEY_HEIGHT,
    KEY_NAME,
    KEY_FLAGS,
    KEY_BORDER_TYPE,
    KEY_VECTOR,
    KEY_SHORTCUT,
    KEY_FLAG
} key_type_t;

// -----------------------------------------------------------------------
// offsets into json_syntax array of hash values found in json.c

typedef enum
{
    JSON_COLON,
    JSON_L_BRACE,
    JSON_R_BRACE,
    JSON_L_BRACKET,
    JSON_R_BRACKET
} json_syntax_t;

// -----------------------------------------------------------------------

typedef struct
{
    int16_t state;       // current state
    void *parent;        // pointer to parent j_state_t
    void *structure;     // pointer to structure being populated
    int16_t struct_type; // type of structure being populated
} j_state_t;

// -----------------------------------------------------------------------

int16_t win_alloc(window_t *win);
void win_pop(window_t *win);
void win_close(window_t *win);
window_t *win_open(int16_t width, int16_t height);
int16_t win_set_pos(window_t *win, int16_t x, int16_t y);
void win_scroll_up(window_t *win);
void win_scroll_dn(window_t *win);
void win_scroll_lt(window_t *win);
void win_scroll_rt(window_t *win);
void win_cup(window_t *win, int16_t x, int16_t y);
void win_set_cx(window_t *win, int16_t x);
void win_set_cy(window_t *win, int16_t y);
void win_crsr_up(window_t *win);
void win_crsr_dn(window_t *win);
void win_crsr_lt(window_t *win);
void win_crsr_rt(window_t *win);
void win_cr(window_t *win);
void win_emit(window_t *win, int32_t c);
cell_t *win_peek_xy(window_t *win, uint16_t x, uint16_t y);
cell_t *win_peek(window_t *win);
void win_clear(window_t *win);
void win_draw_borders(window_t *win);
void win_el(window_t *win);
void win_erase_line(window_t *win, int16_t line);
void win_printf(window_t *win, char *format, ...);
void win_puts(window_t *win, char *s);

// -----------------------------------------------------------------------

screen_t *scr_open(int16_t width, int16_t height);
void scr_close(screen_t *scr);
void scr_win_attach(screen_t *scr, window_t *win);
void scr_win_detach(window_t *win);
void scr_draw_screen(screen_t *scr);
void scr_add_backdrop(screen_t *scr);
int16_t scr_alloc(screen_t *scr);
void init_backdrop(screen_t *scr, window_t *win);

// -----------------------------------------------------------------------

extern char *status_line;
#define MAX_STATUS (40 - 1)

void menu_init(void);
int32_t bar_open(screen_t *scr);
void bar_close(screen_t *scr);
int32_t new_pulldown(screen_t *scr, char *name);
int32_t new_menu_item(screen_t *scr, char *name, menu_fp_t fp,
                      int16_t shortcut);
void bar_draw_text(screen_t *scr);
void bar_populdate_pd(pulldown_t *pd);
void pd_enable(screen_t *scr, char *name);
void pd_disable(screen_t *scr, char *name);

void init_key_handlers(void);
key_handler_t *set_key_action(key_index_t index, key_handler_t *action);
void bar_set_status(char *string);
void bar_clr_status(void);
void bar_draw_status(menu_bar_t *bar);
void alloc_status(void);

// -----------------------------------------------------------------------

int re_switch(const switch_t *s, size_t size, int32_t option);

uint8_t key(void);
uint8_t test_keys(void);
void stuff_key(int8_t c);

// -----------------------------------------------------------------------

void uCurses_init(void);
void uCurses_deInit(void);
void restore_term(void);

// -----------------------------------------------------------------------

int16_t is_wide(int32_t code);
void utf8_emit(int32_t cp);
utf8_encode_t *utf8_encode(int32_t cp);
int8_t utf8_decode(int32_t *cp, char *s);
int16_t utf8_width(char *s);
int16_t utf8_strlen(char *s);
int16_t utf8_strncmp(char *s1, char *s2, int16_t len);
uint8_t utf8_char_length(char *s);

void flush(void);
void c_emit(char c1);
void format(int16_t i);
void parse_format(void);

void apply_attribs(void);

int32_t fnv_hash(char *s);
uint16_t is_keyword(int32_t *table, size_t size, int32_t hash);
void token(void);
int16_t is_token(int32_t *table, size_t size, char *s);
void json_de_tab(char *s, size_t len);
__attribute__((noreturn)) void json_error(const char *s);
void json_new_state_struct(size_t struct_size, int32_t struct_type);
void json_state_value(void);
void json_state_key(void);
void json_state_r_brace(void);
void j_pop(void);
void strip_quotes(int16_t len);
void json_create_ui(char *path, fp_finder_t fp);
void json_build_ui(void);
void populate_parent(void);

// -----------------------------------------------------------------------

void set_ul(void);
void clr_ul(void);

void set_rev(void);
void clr_rev(void);

void set_bold(void);
void clr_bold(void);

void set_fg(int8_t c);
void set_bg(int8_t c);

void set_gray_fg(int8_t c);
void set_gray_bg(int8_t c);

void set_rgb_fg(int8_t r, int8_t g, int8_t b);
void set_rgb_bg(int8_t r, int8_t g, int8_t b);

void set_norm(void);

ti_attrib_t add_attr(uint8_t a, ti_attrib_t attr);

// -----------------------------------------------------------------------
// wrappers for terminfo cursor handling etc

void clock_sleep(int32_t when);

void curoff(void);
void curon(void);
void clear(void);
void hpa(int16_t x);
void cup(int16_t x, int16_t y);
void cud1(void);
void home(void);
void cub1(void);
void cuf1(void);
void cuu1(void);
void dch1(void);
void cud(int16_t n1);
void ich(void);
void cub(int16_t n1);
void cuf(int16_t n1);
void cuu(int16_t n1);
void vpa(int16_t n1);
void cr(void);

__attribute__((noreturn)) void xabort(char *msg);

// -----------------------------------------------------------------------

void braile_8(window_t *win, uint16_t *braile_data, uint8_t *map,
              uint16_t width);
void braile_1(window_t *win, uint16_t *braile_data, uint8_t *map,
              uint16_t width, uint16_t height);
void draw_braile(window_t *win, uint16_t *braile_data);
int16_t braile_xlat(uint8_t chr);

// -----------------------------------------------------------------------
// terminfo name, terminfo strings section offset to format string

#define ti_bell() format(2 >> 1) // NOOOOO!
#define ti_cr() format(4 >> 1)
#define ti_csr() format(6 >> 1)    // change scroll region
#define ti_clear() format(10 >> 1) // clear screen
#define ti_el() format(12 >> 1)    // erase line
#define ti_ed() format(14 >> 1)    // clear to end of screen
#define ti_hpa() format(16 >> 1)   // set cursor horizontal position
#define ti_cup() format(20 >> 1)   // set cursor horizontal / vertical
#define ti_cud1() format(22 >> 1)  // move cursor down 1 line
#define ti_home() format(24 >> 1)  // move cursor to home location
#define ti_civis() format(26 >> 1) // make cursor invisible
#define ti_cub1() format(28 >> 1)  // move cursor back one on line
#define ti_cnorm() format(32 >> 1) // make cursor visible
#define ti_cuf1() format(34 >> 1)  // move cursor forward on line
#define ti_ll() format(36 >> 1)    // move cursor to lower left
#define ti_cuu1() format(38 >> 1)  // move cursor up one line
#define ti_cvvis() format(40 >> 1) // make cursor very visible?
#define ti_dch1() format(42 >> 1)  // delete character
#define ti_dl1() format(44 >> 1)   // delete line
#define ti_bold() format(54 >> 1)  // turn on bold mode
#define ti_smir() format(62 >> 1)  // set mode insert
#define ti_rev() format(68 >> 1)   // turn on reverse video
#define ti_smul() format(72 >> 1)  // turn on underline
#define ti_ech() format(74 >> 1)   // erase chars
#define ti_sgr0() format(78 >> 1)  // blow away all attribs
#define ti_rmir() format(84 >> 1)  // exit insert mode
#define ti_rmul() format(88 >> 1)  // disable underline mode
#define ti_ich1() format(104 >> 1) // insert character
#define ti_il1() format(106 >> 1)  // insert line
#define ti_dch() format(210 >> 1)  // delete 1 character
#define ti_cud() format(214 >> 1)  // move cursor down one
#define ti_ich() format(216 >> 1)  // insert character
#define ti_indn() format(218 >> 1) // scroll forward lines
#define ti_cub() format(222 >> 1)  // move cursor back
#define ti_cuf() format(224 >> 1)  // move cursor forward
#define ti_rin() format(226 >> 1)  // scroll back lines
#define ti_cuu() format(228 >> 1)  // cursor up lines

#define ti_rep() format(242 >> 1) // repeat char

#define ti_rc() format(252 >> 1)    // restore cursor location
#define ti_vpa() format(254 >> 1)   // vertical position absolute
#define ti_sc() format(256 >> 1)    // save cursor location
#define ti_ind() format(258 >> 1)   // scroll up
#define ti_ri() format(260 >> 1)    // scroll down
#define ti_sgr() format(262 >> 1)   // set graphics rendering
#define ti_ht() format(268 >> 1)    // tab to next 8 space stop
#define ti_el1() format(538 >> 1)   // clear to beginning of line
#define ti_rum() format(654 >> 1)   // end reverse character motion
#define ti_setaf() format(718 >> 1) // set fg using ansi
#define ti_setab() format(720 >> 1) // set bg using ansi

#define ti_kbs() format(110 >> 1)   // key backspace
#define ti_kdch1() format(118 >> 1) // delete character key
#define ti_kcud1() format(122 >> 1) // cursor down key
#define ti_kf1() format(132 >> 1)   // f1 key
#define ti_kf10() format(134 >> 1)  // f10 key
#define ti_kf2() format(136 >> 1)   // f2 key
#define ti_kf3() format(138 >> 1)   // f3 key
#define ti_kf4() format(140 >> 1)   // f4 key
#define ti_kf5() format(142 >> 1)   // f5 key
#define ti_kf6() format(144 >> 1)   // f6 key
#define ti_kf7() format(146 >> 1)   // f7 key
#define ti_kf8() format(148 >> 1)   // f8 key
#define ti_kf9() format(150 >> 1)   // f9 key
#define ti_khome() format(152 >> 1) // home key
#define ti_kich1() format(154 >> 1) // insert character key
#define ti_kcub1() format(158 >> 1) // cursor left key
#define ti_knp() format(162 >> 1)   // next page key
#define ti_kpp() format(164 >> 1)   // previous page key
#define ti_kcuf1() format(166 >> 1) // cursor right key
#define ti_kcuu1() format(174 >> 1) // cursor up key
#define ti_rmkx() format(176 >> 1)  // leave keyboard transmit mode
#define ti_smkx() format(178 >> 1)  // enter keyboard transmit mode
#define ti_kcbt() format(296 >> 1)  // backtab key
#define ti_kend() format(328 >> 1)  // key end
#define ti_kent() format(330 >> 1)  // key enter
#define ti_kDC() format(382 >> 1)   // shifted delete character key
#define ti_kEND() format(388 >> 1)  // shifted end key
#define ti_kHOM() format(398 >> 1)  // shifted home key
#define ti_kIC() format(400 >> 1)   // shifted insert character key
#define ti_kLFT() format(402 >> 1)  // shifted cursor left
#define ti_kNXT() format(408 >> 1)  // shifted next key
#define ti_kPRV() format(412 >> 1)  // shifted previous key
#define ti_kRIT() format(420 >> 1)  // shifted cursor right
#define ti_kf11() format(432 >> 1)  // f11 function key
#define ti_kf12() format(434 >> 1)  // f12 function key

#define ti_kmous() format(710 >> 1) // mouse event has occurred

// -----------------------------------------------------------------------

#endif // UCURSES_H

// =======================================================================
