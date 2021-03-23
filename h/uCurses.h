// uCurses.h
// -----------------------------------------------------------------------

    #pragma once

// -----------------------------------------------------------------------

#include <inttypes.h>

#include "tui.h"

// -----------------------------------------------------------------------

#define MAX_PARAM 9

// -----------------------------------------------------------------------

typedef void (*fp_t)(void *x);

// -----------------------------------------------------------------------

extern uint64_t params[MAX_PARAM];

// --------------------------------------------------------------------------

typedef struct
{
    uint8_t len;
    uint8_t str[4];
    int width;
} utf8_encode_t;

// -----------------------------------------------------------------------

void uCurses_init(void);
void format(uint16_t i);
void do_parse_format(void);
void utf8_emit(uint32_t cp);
utf8_encode_t *utf8_encode(uint32_t cp);
uint8_t utf8_decode(uint32_t *cp, char *s);
uint16_t utf8_width(char *s);
uint16_t utf8_strlen(char *s);
uint16_t utf8_strncmp(char *s1, char *s2, uint16_t len);
void pd_enable(screen_t *scr, char *name);
void pd_disable(screen_t *scr, char *name);

void apply_attribs(void);
void flush(void);
void c_emit(uint8_t c1);

// -----------------------------------------------------------------------

#define DEADCODE 0xdeadc0de

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

// =======================================================================
