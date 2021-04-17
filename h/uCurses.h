// uCurses.h
// -----------------------------------------------------------------------

  #ifndef UCURSES_H
  #define UCURSES_H

// -----------------------------------------------------------------------

#include <inttypes.h>
#include <aio.h>

#include "list.h"

// -----------------------------------------------------------------------

    #define MAX_PARAM 9
    extern uint64_t params[MAX_PARAM];

// -----------------------------------------------------------------------
// utf-8 codepoint for 'blank' char used in screen backdrop windows

    #define SOLID  0x2592
    #define SHADOW 0x2591

    #define MENU_DISABLED 1
    #define MAX_MENU_ITEMS 10
    #define DEADCODE 0xdeadc0de

// -----------------------------------------------------------------------

typedef void (*opt_t)(void);       // re_switch vectors
typedef void (*menu_fp_t)(void);   // menu option vectors
typedef void (*fp_t)(void *x);

typedef menu_fp_t (*fp_finder_t)(uint32_t hash);

// -----------------------------------------------------------------------

typedef void key_handler_t(void);

typedef enum
{
    K_ENT,  K_CUU1, K_CUD1, K_CUB1, K_CUF1, K_BS,  K_BS2,
    K_DCH1, K_ICH1, K_HOME, K_END,  K_KNP,  K_KPP, K_F1,
    K_F2,   K_F3,   K_F4,   K_F5,   K_F6,   K_F7,  K_F8,
    K_F9,   K_F10,  K_F11,  K_f12
} key_index_t;

// -----------------------------------------------------------------------
// c switch statements are FUGLY

typedef struct
{
    uint32_t option;
    opt_t vector;
} switch_t;

// -----------------------------------------------------------------------

#define win_clr_attr(win, attr) win->attrs[ATTR] &= ~attr

#define win_set_ul(win)    win_set_attr(win, UNDERLINE)
#define win_set_rev(win)   win_set_attr(win, REVERSE)
#define win_set_bold(win)  win_set_attr(win, BOLD)
#define win_set_blink(win) win_set_attr(win, BLINK)
#define win_clr_ul(win)    win_clr_attr(win, UNDERLINE)
#define win_clr_rev(win)   win_clr_attr(win, REVERSE)
#define win_clr_bold(win)  win_clr_attr(win, BOLD)
#define win_clr_blink(win) win_clr_attr(win, BLINK)

#define win_set_boxed(win)   win->flags |= WIN_BOXED
#define win_set_locked(win)  win->flags |= WIN_LOCKED
#define win_set_filled(win)  win->flags |= WIN_FILLED

#define win_clr_boxed(win)   win->flags &= ~WIN_BOXED
#define win_clr_locked(win)  win->flags &= ~WIN_LOCKED
#define win_clr_filled(win)  win->flags &= ~WIN_FILLED

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
    WIN_BOXED      = 1,     // has a border
    WIN_LOCKED     = 2,     // scroll locked
    WIN_FILLED     = 4      // backfilled with SOLID character
} win_flags_t;

// --------------------------------------------------------------------------

typedef struct
{
    uint8_t len;
    uint8_t str[4];
    int width;
} utf8_encode_t;

// -----------------------------------------------------------------------
// structure of each cell of a window / screen

typedef struct
{
    uint8_t attrs[8];       // bold, blink, underline, gray scale, rgb
    uint32_t code;          // utf-8 codepoint
} cell_t;

// -----------------------------------------------------------------------

typedef struct
{
    char *name;
    uint16_t flags;         // disable flags etc
    menu_fp_t fp;           // function to execute
    uint16_t shortcut;      // keyboard shortcut
} menu_item_t;

// -----------------------------------------------------------------------

typedef struct
{
    char *name;             // menu bar name for this pulldown menu
    uint16_t width;         // width of widest item in pulldown menu
    uint16_t flags;         // masks for enabled/disabled etc
    uint16_t which;         // current selected item
    uint16_t xco;           // x coordinate of menu window
    uint16_t count;
    // not a linked list of sub items. max 10
    menu_item_t *items[MAX_MENU_ITEMS];
    void *window;           // this is a window_t honest!
    uint8_t attr[8];        // attribs for pulldown menu border
    uint8_t normal[8];      // attribs for non selected menu items
    uint8_t selected[8];    // atrribs for selected menu item
    uint8_t disabled[8];    // attribs for disabled meny items
} pulldown_t;

// -----------------------------------------------------------------------

typedef struct
{
    void *window;             // fwd ref to window_t * grrr (c sucks)
    uint16_t xco;             // x coordinate of next pulldown
    uint16_t active;          // 0 = not active
    uint16_t which;           // which pulldown item is active
    uint16_t count;           // number of pulldowns defined
    pulldown_t *items[MAX_MENU_ITEMS];
    uint8_t normal[8];        // attribs for non selected menu bar items
    uint8_t selected[8];      // attribs for selected menu bar items
    uint8_t disabled[8];      // attribs for disabled menu bar items
} menu_bar_t;

// -----------------------------------------------------------------------

typedef struct
{
    list_t windows;         // linked list of windows
    menu_bar_t *menu_bar;
    cell_t *buffer1;        // screen buffer 1 and 2
    cell_t *buffer2;
    void *backdrop;         // really a window_t *
    uint16_t width;         // screen dimensions
    uint16_t height;
    uint16_t cx;            // cursor corrdinates within screen
    uint16_t cy;
} screen_t;

    extern screen_t *active_screen;

// -----------------------------------------------------------------------

typedef struct
{
    node_t      *links;
    cell_t      *buffer;
    screen_t    *screen;
    win_flags_t flags;
    uint32_t    blank;         // window fill character for backdrop windows
    uint16_t    width;         // window dimensions
    uint16_t    height;
    uint16_t    xco;           // window x/y coordinat within screen
    uint16_t    yco;
    uint16_t    cx;            // cursor position within window
    uint16_t    cy;
    uint16_t    bdr_type;
    uint8_t     attrs[8];      // bold blink underline, gray scale, rgb etc
    uint8_t     old_attrs[8];  // previous state..
    uint8_t     bdr_attrs[8];  // likewise for the windows border if it has
} window_t;

// -----------------------------------------------------------------------

typedef enum
{
    BLACK, RED, GREEN, BROWN,
    BLUE, MAGENTA, CYAN, WHITE,
    GRAY, LT_RED, LT_GREEN, YELLOW,
    LT_BLUE, LT_MAGENTA, LT_CYAN, LT_WHITE
} ti_color_t;

// -----------------------------------------------------------------------

typedef enum
{
    TI_UNDERLINE,  TI_REVERSE,   TI_BOLD,     TI_BLINK,
    TI_FG_RGB,     TI_BG_RGB,    TI_FG_GRAY,  TI_BG_GRAY,

    UNDERLINE = (1 << TI_UNDERLINE),
    REVERSE   = (1 << TI_REVERSE),
    BOLD      = (1 << TI_BOLD),
    BLINK     = (1 << TI_BLINK),

    FG_RGB    = (1 << TI_FG_RGB),
    BG_RGB    = (1 << TI_BG_RGB),
    FG_GRAY   = (1 << TI_FG_GRAY),
    BG_GRAY   = (1 << TI_BG_GRAY),
} ti_attrib_t;

// -----------------------------------------------------------------------
// indicies into attributes array

enum
{
    ATTR,                   // attribute flags
    FG_R,                   // rgb fg red
    FG_G,                   // rgb fg green
    FG_B,                   // rgb fg blue
    BG_R,                   // rgb bg red
    FG,    BG_G = 5,        // foreground or rgb bg green
    BG,    BG_B = 6,        // background or rgb bg blue
    UNUSED
};

// -----------------------------------------------------------------------

#define MAX_LINE_LEN 128
#define TOKEN_LEN 32

typedef struct
{
    uint16_t state;         // current state
    void *parent;           // pointer to parent j_state_t
    void *structure;        // pointer to structure being populated
    uint16_t struct_type;   // type of structure being populated
} j_state_t;

// -----------------------------------------------------------------------

void win_pop(window_t *win);
void win_close(window_t *win);
window_t *win_open(uint16_t width, uint16_t height);
uint16_t win_set_pos(window_t *win, uint16_t x, uint16_t y);
void win_set_gray_fg(window_t *win, uint8_t c);
void win_set_gray_bg(window_t *win, uint8_t c);
void win_set_rgb_fg(window_t *win, uint8_t r, uint8_t g, uint8_t b);
void win_set_rgb_bg(window_t *win, uint8_t r, uint8_t g, uint8_t b);
void win_set_fg(window_t *win, uint8_t color);
void win_set_bg(window_t *win, uint8_t color);
void win_scroll_up(window_t *win);
void win_scroll_dn(window_t *win);
void win_scroll_lt(window_t *win);
void win_scroll_rt(window_t *win);
void win_cup(window_t *win, uint16_t x, uint16_t y);
void win_set_cx(window_t *win, uint16_t x);
void win_set_cy(window_t *win, uint16_t y);
void win_crsr_up(window_t *win);
void win_crsr_dn(window_t *win);
void win_crsr_lt(window_t *win);
void win_crsr_rt(window_t *win);
void win_cr(window_t *win);
void win_emit(window_t *win, uint32_t c);
void win_clear(window_t *win);
void win_draw_borders(window_t *win);
void win_el(window_t *win);
void win_erase_line(window_t *win, uint16_t line);
void win_printf(window_t *win, char* format, ...);
void win_puts(window_t *win, char *s);

// -----------------------------------------------------------------------

screen_t *scr_open(uint16_t width, uint16_t height);
void scr_close(screen_t *scr);
void scr_win_attach(screen_t *scr, window_t *win);
void scr_win_detach(window_t *win);
void scr_cup(screen_t *scr, uint16_t x, uint16_t y);
void scr_draw_screen(screen_t *scr);
void scr_add_backdrop(screen_t *scr);

// -----------------------------------------------------------------------

void menu_init(void);
uint32_t bar_open(screen_t *scr);
void bar_close(screen_t *scr);
uint32_t new_pulldown(screen_t *scr, char *name);
uint32_t new_menu_item(screen_t *scr, char *name, menu_fp_t fp,
    uint16_t shortcut);
void bar_draw_text(screen_t *scr);
void bar_populdate_pd(pulldown_t *pd);
void pd_enable(screen_t *scr, char *name);
void pd_disable(screen_t *scr, char *name);

void init_key_handlers(void);
key_handler_t *set_key_action(key_index_t index, key_handler_t* action);
void stuff_key(uint8_t c);

// -----------------------------------------------------------------------

int re_switch(const switch_t *s, size_t size, uint32_t option);
uint8_t key(void);

// -----------------------------------------------------------------------

void uCurses_init(void);
void uCurses_deInit(void);

// -----------------------------------------------------------------------

uint16_t is_wide(uint32_t code);
void utf8_emit(uint32_t cp);
utf8_encode_t *utf8_encode(uint32_t cp);
uint8_t utf8_decode(uint32_t *cp, char *s);
uint16_t utf8_width(char *s);
uint16_t utf8_strlen(char *s);
uint16_t utf8_strncmp(char *s1, char *s2, uint16_t len);
uint8_t utf8_char_length(char *s);

void flush(void);
void c_emit(uint8_t c1);
void format(uint16_t i);
void parse_format(void);

void apply_attribs(void);

uint32_t fnv_hash(char *s);
uint16_t is_keyword(uint32_t *table, size_t size, uint32_t hash);
void token(void);
uint16_t is_token(uint32_t *table, size_t size, char *s);
void json_de_tab(char *s, size_t len);
void json_error(char *s);

// -----------------------------------------------------------------------

void set_ul(void);
void set_rev(void);
void set_bold(void);
void set_blink(void);

void clr_ul(void);
void clr_rev(void);
void clr_bold(void);
void clr_blink(void);

void set_gray_fg(uint8_t c);
void set_gray_bg(uint8_t c);

void set_rgb_fg(uint8_t r, uint8_t g, uint8_t b);
void set_rgb_bg(uint8_t r, uint8_t g, uint8_t b);

void set_fg(uint8_t c);
void set_bg(uint8_t c);

void set_norm(void);

// -----------------------------------------------------------------------
// wrappers for terminfo cursor handling etc

void curoff(void);
void curon(void);
void clear(void);
void hpa(uint16_t x);
void cup(uint16_t x, uint16_t y);
void cud1(void);
void home(void);
void cub1(void);
void cuf1(void);
void cuu1(void);
void dch1(void);
void cud(uint16_t n1);
void ich(void);
void cub(uint16_t n1);
void cuf(uint16_t n1);
void cuu(uint16_t n1);
void vpa(uint16_t n1);
void cr(void);

// -----------------------------------------------------------------------
//      +--- terminfo name
//      |                    +--terminfo strings section offset to
//      v                    v  to format string

#define ti_bell()    format(  2 >> 1)  // NOOOOO!
#define ti_cr()      format(  4 >> 1)
#define ti_csr()     format(  6 >> 1)  // change scroll region
#define ti_clear()   format( 10 >> 1)  // clear screen
#define ti_el()      format( 12 >> 1)  // erase line
#define ti_ed()      format( 14 >> 1)  // clear to end of screen
#define ti_hpa()     format( 16 >> 1)  // set cursor horizontal position
#define ti_cup()     format( 20 >> 1)  // set cursor horizontal / vertical
#define ti_cud1()    format( 22 >> 1)  // move cursor down 1 line
#define ti_home()    format( 24 >> 1)  // move cursor to home location
#define ti_civis()   format( 26 >> 1)  // make cursor invisible
#define ti_cub1()    format( 28 >> 1)  // move cursor back one on line
#define ti_cnorm()   format( 32 >> 1)  // make cursor visible
#define ti_cuf1()    format( 34 >> 1)  // move cursor forward on line
#define ti_ll()      format( 36 >> 1)  // move cursor to lower left
#define ti_cuu1()    format( 38 >> 1)  // move cursor up one line
#define ti_cvvis()   format( 40 >> 1)  // make cursor very visible?
#define ti_dch1()    format( 42 >> 1)  // delete character
#define ti_dl1()     format( 44 >> 1)  // delete line
#define ti_blink()   format( 52 >> 1)  // NOOOOO !
#define ti_bold()    format( 54 >> 1)  // turn on bold mode
#define ti_smir()    format( 62 >> 1)  // set mode insert
#define ti_rev()     format( 68 >> 1)  // turn on reverse video
#define ti_smul()    format( 72 >> 1)  // turn on underline
#define ti_ech()     format( 74 >> 1)  // erase chars
#define ti_sgr0()    format( 78 >> 1)  // blow away all attribs
#define ti_rmir()    format( 84 >> 1)  // exit insert mode
#define ti_rmul()    format( 88 >> 1)  // disable underline mode
#define ti_ich1()    format(104 >> 1)  // insert character
#define ti_il1()     format(106 >> 1)  // insert line
#define ti_dch()     format(210 >> 1)  // delete 1 character
#define ti_cud()     format(214 >> 1)  // move cursor down one
#define ti_ich()     format(216 >> 1)  // insert character
#define ti_indn()    format(218 >> 1)  // scroll forward lines
#define ti_cub()     format(222 >> 1)  // move cursor back
#define ti_cuf()     format(224 >> 1)  // move cursor forward
#define ti_rin()     format(226 >> 1)  // scroll back lines
#define ti_cuu()     format(228 >> 1)  // cursor up lines

#define ti_rep()     format(242 >> 1)  // repeat char

#define ti_rc()      format(252 >> 1)  // restore cursor location
#define ti_vpa()     format(254 >> 1)  // vertical position absolute
#define ti_sc()      format(256 >> 1)  // save cursor location
#define ti_ind()     format(258 >> 1)  // scroll up
#define ti_ri()      format(260 >> 1)  // scroll down
#define ti_sgr()     format(262 >> 1)  // set graphics rendering
#define ti_ht()      format(268 >> 1)  // tab to next 8 space stop
#define ti_el1()     format(538 >> 1)  // clear to beginning of line
#define ti_rum()     format(654 >> 1)  // end reverse character motion
#define ti_setaf()   format(718 >> 1)  // set fg using ansi
#define ti_setab()   format(720 >> 1)  // set bg using ansi

#define ti_kbs()     format(110 >> 1)  // key backspace
#define ti_kdch1()   format(118 >> 1)  // delete character key
#define ti_kcud1()   format(122 >> 1)  // cursor down key
#define ti_kf1()     format(132 >> 1)  // f1 key
#define ti_kf10()    format(134 >> 1)  // f10 key
#define ti_kf2()     format(136 >> 1)  // f2 key
#define ti_kf3()     format(138 >> 1)  // f3 key
#define ti_kf4()     format(140 >> 1)  // f4 key
#define ti_kf5()     format(142 >> 1)  // f5 key
#define ti_kf6()     format(144 >> 1)  // f6 key
#define ti_kf7()     format(146 >> 1)  // f7 key
#define ti_kf8()     format(148 >> 1)  // f8 key
#define ti_kf9()     format(150 >> 1)  // f9 key
#define ti_khome()   format(152 >> 1)  // home key
#define ti_kich1()   format(154 >> 1)  // insert character key
#define ti_kcub1()   format(158 >> 1)  // cursor left key
#define ti_knp()     format(162 >> 1)  // next page key
#define ti_kpp()     format(164 >> 1)  // previous page key
#define ti_kcuf1()   format(166 >> 1)  // cursor right key
#define ti_kcuu1()   format(174 >> 1)  // cursor up key
#define ti_rmkx()    format(176 >> 1)  // leave keyboard transmit mode
#define ti_smkx()    format(178 >> 1)  // enter keyboard transmit mode
#define ti_kcbt()    format(296 >> 1)  // backtab key
#define ti_kend()    format(328 >> 1)  // key end
#define ti_kent()    format(330 >> 1)  // key enter
#define ti_kDC()     format(382 >> 1)  // shifted delete character key
#define ti_kEND()    format(388 >> 1)  // shifted end key
#define ti_kHOM()    format(398 >> 1)  // shifted home key
#define ti_kIC()     format(400 >> 1)  // shifted insert character key
#define ti_kLFT()    format(402 >> 1)  // shifted cursor left
#define ti_kNXT()    format(408 >> 1)  // shifted next key
#define ti_kPRV()    format(412 >> 1)  // shifted previous key
#define ti_kRIT()    format(420 >> 1)  // shifted cursor right
#define ti_kf11()    format(432 >> 1)  // f11 function key
#define ti_kf12()    format(434 >> 1)  // f12 function key

#define ti_kmous()   format(710 >> 1)  // mouse event has occurred

// -----------------------------------------------------------------------

  #endif  // UCURSES_H

// =======================================================================
