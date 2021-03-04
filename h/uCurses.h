// uCurses.h
// -----------------------------------------------------------------------

    #pragma once

// -----------------------------------------------------------------------

#include <inttypes.h>

// -----------------------------------------------------------------------

#define MAX_PARAM 9

// -----------------------------------------------------------------------

typedef void (*fp_t)(void *x);

// -----------------------------------------------------------------------

extern uint64_t params[MAX_PARAM];

// -----------------------------------------------------------------------

void uCurses_init(void);
void format(uint16_t i);
void do_parse_format(void);
void utf8_emit(uint32_t code);
void apply_attribs(void);

// -----------------------------------------------------------------------
//      +--- terminfo name
//      |                    +--terminfo strings section offset to
//      v                    v  to format string
#define ti_bell()    format(  2 >> 1)
#define ti_cr()      format(  4 >> 1)
#define ti_csr()     format(  6 >> 1)
#define ti_clear()   format( 10 >> 1)
#define ti_el()      format( 12 >> 1)
#define ti_ed()      format( 14 >> 1)
#define ti_hpa()     format( 16 >> 1)
#define ti_cup()     format( 20 >> 1)
#define ti_cud1()    format( 22 >> 1)
#define ti_home()    format( 24 >> 1)
#define ti_civis()   format( 26 >> 1)
#define ti_cub1()    format( 28 >> 1)
#define ti_cnorm()   format( 32 >> 1)
#define ti_cuf1()    format( 34 >> 1)
#define ti_ll()      format( 36 >> 1)
#define ti_cuu1()    format( 38 >> 1)
#define ti_cvvis()   format( 40 >> 1)
#define ti_dch1()    format( 42 >> 1)
#define ti_dl1()     format( 44 >> 1)
#define ti_smacs()   format( 50 >> 1)
#define ti_blink()   format( 52 >> 1)
#define ti_bold()    format( 54 >> 1)
#define ti_smcup()   format( 56 >> 1)
#define ti_dim()     format( 60 >> 1)
#define ti_smir()    format( 62 >> 1)
#define ti_rev()     format( 68 >> 1)
#define ti_smso()    format( 70 >> 1)
#define ti_smul()    format( 72 >> 1)
#define ti_ech()     format( 74 >> 1)
#define ti_rmacs()   format( 76 >> 1)
#define ti_sgr0()    format( 78 >> 1)
#define ti_rmcup()   format( 80 >> 1)
#define ti_rmir()    format( 84 >> 1)
#define ti_rmso()    format( 86 >> 1)
#define ti_rmul()    format( 88 >> 1)
#define ti_is1()     format( 96 >> 1)
#define ti_is2()     format( 98 >> 1)
#define ti_is3()     format(100 >> 1)
#define ti_ich1()    format(104 >> 1)
#define ti_il1()     format(106 >> 1)
#define ti_dch()     format(210 >> 1)
#define ti_cud()     format(214 >> 1)
#define ti_ich()     format(216 >> 1)
#define ti_indn()    format(218 >> 1)
#define ti_cub()     format(222 >> 1)
#define ti_cuf()     format(224 >> 1)
#define ti_rin()     format(226 >> 1)
#define ti_cuu()     format(228 >> 1)
#define ti_rc()      format(252 >> 1)
#define ti_vpa()     format(254 >> 1)
#define ti_sc()      format(256 >> 1)
#define ti_ind()     format(258 >> 1)
#define ti_ri()      format(260 >> 1)
#define ti_sgr()     format(262 >> 1)
#define ti_hts()     format(264 >> 1)
#define ti_ht()      format(268 >> 1)
#define ti_enacs()   format(310 >> 1)
#define ti_el1()     format(538 >> 1)
#define ti_op()      format(594 >> 1)
#define ti_rum()     format(654 >> 1)
#define ti_scs()     format(678 >> 1)
#define ti_setaf()   format(718 >> 1)
#define ti_setab()   format(720 >> 1)

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
