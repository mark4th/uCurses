// terminfo.h
// -----------------------------------------------------------------------

#ifndef TERMINFO_H
#define TERMINFO_H

// -----------------------------------------------------------------------

#include <stdint.h>

#include "uC_parse.h"

// -----------------------------------------------------------------------

typedef struct
{
    int8_t *ti_map;             // memory mapped address of terminfo file
    int16_t ti_size;            // size of memory mapping

    char    *ti_names;          // pointer to term names
    int8_t  *ti_bool;           // pointer to terminfo flags
    int16_t *ti_numbers;        // pointer to terminfo numbers
    int16_t *ti_strings;
    char    *ti_table;

    int8_t wide;                // numbers item size size shift factor
} ti_file_t;

// -----------------------------------------------------------------------
// terminfo name, terminfo strings section offset to format string

#define ti_bell()  uC_format(2 >> 1)    // NOOOOO!
#define ti_cr()    uC_format(4 >> 1)
#define ti_csr()   uC_format(6 >> 1)    // change scroll region
#define ti_clear() uC_format(10 >> 1)   // clear screen
#define ti_el()    uC_format(12 >> 1)   // erase line
#define ti_ed()    uC_format(14 >> 1)   // clear to end of screen
#define ti_hpa()   uC_format(16 >> 1)   // set cursor horizontal position
#define ti_cup()   uC_format(20 >> 1)   // set cursor horizontal / vertical
#define ti_cud1()  uC_format(22 >> 1)   // move cursor down 1 line
#define ti_home()  uC_format(24 >> 1)   // move cursor to home location
#define ti_civis() uC_format(26 >> 1)   // make cursor invisible
#define ti_cub1()  uC_format(28 >> 1)   // move cursor back one on line
#define ti_cnorm() uC_format(32 >> 1)   // make cursor visible
#define ti_cuf1()  uC_format(34 >> 1)   // move cursor forward on line
#define ti_ll()    uC_format(36 >> 1)   // move cursor to lower left
#define ti_cuu1()  uC_format(38 >> 1)   // move cursor up one line
#define ti_cvvis() uC_format(40 >> 1)   // make cursor very visible?
#define ti_dch1()  uC_format(42 >> 1)   // delete character
#define ti_dl1()   uC_format(44 >> 1)   // delete line
#define ti_bold()  uC_format(54 >> 1)   // turn on bold mode
#define ti_smir()  uC_format(62 >> 1)   // set mode insert
#define ti_rev()   uC_format(68 >> 1)   // turn on reverse video
#define ti_smul()  uC_format(72 >> 1)   // turn on underline
#define ti_ech()   uC_format(74 >> 1)   // erase chars
#define ti_sgr0()  uC_format(78 >> 1)   // blow away all attribs
#define ti_rmir()  uC_format(84 >> 1)   // exit insert mode
#define ti_rmul()  uC_format(88 >> 1)   // disable underline mode
#define ti_ich1()  uC_format(104 >> 1)  // insert character
#define ti_il1()   uC_format(106 >> 1)  // insert line
#define ti_dch()   uC_format(210 >> 1)  // delete 1 character
#define ti_cud()   uC_format(214 >> 1)  // move cursor down one
#define ti_ich()   uC_format(216 >> 1)  // insert character
#define ti_indn()  uC_format(218 >> 1)  // scroll forward lines
#define ti_cub()   uC_format(222 >> 1)  // move cursor back
#define ti_cuf()   uC_format(224 >> 1)  // move cursor forward
#define ti_rin()   uC_format(226 >> 1)  // scroll back lines
#define ti_cuu()   uC_format(228 >> 1)  // cursor up lines

#define ti_rep()   uC_format(242 >> 1)  // repeat char

#define ti_rc()    uC_format(252 >> 1)  // restore cursor location
#define ti_vpa()   uC_format(254 >> 1)  // vertical position absolute
#define ti_sc()    uC_format(256 >> 1)  // save cursor location
#define ti_ind()   uC_format(258 >> 1)  // scroll up
#define ti_ri()    uC_format(260 >> 1)  // scroll down
#define ti_sgr()   uC_format(262 >> 1)  // set graphics rendering
#define ti_ht()    uC_format(268 >> 1)  // tab to next 8 space stop
#define ti_el1()   uC_format(538 >> 1)  // clear to beginning of line
#define ti_rum()   uC_format(654 >> 1)  // end reverse character motion
#define ti_setaf() uC_format(718 >> 1)  // set fg using ansi
#define ti_setab() uC_format(720 >> 1)  // set bg using ansi

#define ti_kbs()   uC_format(110 >> 1)  // key backspace
#define ti_kdch1() uC_format(118 >> 1)  // delete character key
#define ti_kcud1() uC_format(122 >> 1)  // cursor down key
#define ti_kf1()   uC_format(132 >> 1)  // f1 key
#define ti_kf10()  uC_format(134 >> 1)  // f10 key
#define ti_kf2()   uC_format(136 >> 1)  // f2 key
#define ti_kf3()   uC_format(138 >> 1)  // f3 key
#define ti_kf4()   uC_format(140 >> 1)  // f4 key
#define ti_kf5()   uC_format(142 >> 1)  // f5 key
#define ti_kf6()   uC_format(144 >> 1)  // f6 key
#define ti_kf7()   uC_format(146 >> 1)  // f7 key
#define ti_kf8()   uC_format(148 >> 1)  // f8 key
#define ti_kf9()   uC_format(150 >> 1)  // f9 key
#define ti_khome() uC_format(152 >> 1)  // home key
#define ti_kich1() uC_format(154 >> 1)  // insert character key
#define ti_kcub1() uC_format(158 >> 1)  // cursor left key
#define ti_knp()   uC_format(162 >> 1)  // next page key
#define ti_kpp()   uC_format(164 >> 1)  // previous page key
#define ti_kcuf1() uC_format(166 >> 1)  // cursor right key
#define ti_kcuu1() uC_format(174 >> 1)  // cursor up key
#define ti_rmkx()  uC_format(176 >> 1)  // leave keyboard transmit mode
#define ti_smkx()  uC_format(178 >> 1)  // enter keyboard transmit mode
#define ti_kcbt()  uC_format(296 >> 1)  // backtab key
#define ti_kend()  uC_format(328 >> 1)  // key end
#define ti_kent()  uC_format(330 >> 1)  // key enter
#define ti_kDC()   uC_format(382 >> 1)  // shifted delete character key
#define ti_kEND()  uC_format(388 >> 1)  // shifted end key
#define ti_kHOM()  uC_format(398 >> 1)  // shifted home key
#define ti_kIC()   uC_format(400 >> 1)  // shifted insert character key
#define ti_kLFT()  uC_format(402 >> 1)  // shifted cursor left
#define ti_kNXT()  uC_format(408 >> 1)  // shifted next key
#define ti_kPRV()  uC_format(412 >> 1)  // shifted previous key
#define ti_kRIT()  uC_format(420 >> 1)  // shifted cursor right
#define ti_kf11()  uC_format(432 >> 1)  // f11 function key
#define ti_kf12()  uC_format(434 >> 1)  // f12 function key

#define ti_kmous() uC_format(710 >> 1)  // mouse event has occurred

// -----------------------------------------------------------------------

API void uC_curoff(void);
API void uC_curon(void);
API void uC_clear(void);
API void uC_hpa(int16_t x);
API void uC_cup(int16_t x, int16_t y);
API void uC_cud1(void);
API void uC_home(void);
API void uC_cub1(void);
API void uC_cuf1(void);
API void uC_cuu1(void);
API void uC_dch1(void);
API void uC_cud(int16_t n1);
API void uC_ich(void);
API void uC_cub(int16_t n1);
API void uC_cuf(int16_t n1);
API void uC_cuu(int16_t n1);
API void uC_vpa(int16_t y1);
API void uC_cr(void);
API void uC_smkx(void);
API void uC_rmkx(void);

// -----------------------------------------------------------------------

#endif // TERMINFO_H

// =======================================================================
