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
void _format(uint16_t i);
void do_flush(void *unused);
void do_parse_format(void);
void utf8_emit(uint32_t code);
void set_attribs(void);

extern fp_t flush;

// -----------------------------------------------------------------------

#define ti_bell()    _format(  2 >> 1)
#define ti_cr()      _format(  4 >> 1)
#define ti_csr()     _format(  6 >> 1)
#define ti_clear()   _format( 10 >> 1)
#define ti_el()      _format( 12 >> 1)
#define ti_ed()      _format( 14 >> 1)
#define ti_hpa()     _format( 16 >> 1)
#define ti_cup()     _format( 20 >> 1)
#define ti_cud1()    _format( 22 >> 1)
#define ti_home()    _format( 24 >> 1)
#define ti_civis()   _format( 26 >> 1)
#define ti_cub1()    _format( 28 >> 1)
#define ti_cnorm()   _format( 32 >> 1)
#define ti_cuf1()    _format( 34 >> 1)
#define ti_ll()      _format( 36 >> 1)
#define ti_cuu1()    _format( 38 >> 1)
#define ti_cvvis()   _format( 40 >> 1)
#define ti_dch1()    _format( 42 >> 1)
#define ti_dl1()     _format( 44 >> 1)
#define ti_smacs()   _format( 50 >> 1)
#define ti_blink()   _format( 52 >> 1)
#define ti_bold()    _format( 54 >> 1)
#define ti_smcup()   _format( 56 >> 1)
#define ti_dim()     _format( 60 >> 1)
#define ti_smir()    _format( 62 >> 1)
#define ti_rev()     _format( 68 >> 1)
#define ti_smso()    _format( 70 >> 1)
#define ti_smul()    _format( 72 >> 1)
#define ti_ech()     _format( 74 >> 1)
#define ti_rmacs()   _format( 76 >> 1)
#define ti_sgr0()    _format( 78 >> 1)
#define ti_rmcup()   _format( 80 >> 1)
#define ti_rmir()    _format( 84 >> 1)
#define ti_rmso()    _format( 86 >> 1)
#define ti_rmul()    _format( 88 >> 1)
#define ti_is1()     _format( 96 >> 1)
#define ti_is2()     _format( 98 >> 1)
#define ti_is3()     _format(100 >> 1)
#define ti_ich1()    _format(104 >> 1)
#define ti_il1()     _format(106 >> 1)
#define ti_dch()     _format(210 >> 1)
#define ti_cud()     _format(214 >> 1)
#define ti_ich()     _format(216 >> 1)
#define ti_indn()    _format(218 >> 1)
#define ti_cub()     _format(222 >> 1)
#define ti_cuf()     _format(224 >> 1)
#define ti_rin()     _format(226 >> 1)
#define ti_cuu()     _format(228 >> 1)
#define ti_rc()      _format(252 >> 1)
#define ti_vpa()     _format(254 >> 1)
#define ti_sc()      _format(256 >> 1)
#define ti_ind()     _format(258 >> 1)
#define ti_ri()      _format(260 >> 1)
#define ti_sgr()     _format(262 >> 1)
#define ti_hts()     _format(264 >> 1)
#define ti_ht()      _format(268 >> 1)
#define ti_enacs()   _format(310 >> 1)
#define ti_el1()     _format(538 >> 1)
#define ti_op()      _format(594 >> 1)
#define ti_rum()     _format(654 >> 1)
#define ti_scs()     _format(678 >> 1)
#define ti_setaf()   _format(718 >> 1)
#define ti_setab()   _format(720 >> 1)

#define ti_kbs()     _format(110 >> 1)  // key backspace
#define ti_kdch1()   _format(118 >> 1)  // delete character key
#define ti_kcud1()   _format(122 >> 1)  // cursor down key
#define ti_kf1()     _format(132 >> 1)  // f1 key
#define ti_kf10()    _format(134 >> 1)  // f10 key
#define ti_kf2()     _format(136 >> 1)  // f2 key
#define ti_kf3()     _format(138 >> 1)  // f3 key
#define ti_kf4()     _format(140 >> 1)  // f4 key
#define ti_kf5()     _format(142 >> 1)  // f5 key
#define ti_kf6()     _format(144 >> 1)  // f6 key
#define ti_kf7()     _format(146 >> 1)  // f7 key
#define ti_kf8()     _format(148 >> 1)  // f8 key
#define ti_kf9()     _format(150 >> 1)  // f9 key
#define ti_khome()   _format(152 >> 1)  // home key
#define ti_kich1()   _format(154 >> 1)  // insert character key
#define ti_kcub1()   _format(158 >> 1)  // cursor left key
#define ti_knp()     _format(162 >> 1)  // next page key
#define ti_kpp()     _format(164 >> 1)  // previous page key
#define ti_kcuf1()   _format(166 >> 1)  // cursor right key
#define ti_kcuu1()   _format(174 >> 1)  // cursor up key
#define ti_rmkx()    _format(176 >> 1)  // leave keyboard transmit mode
#define ti_smkx()    _format(178 >> 1)  // enter keyboard transmit mode
#define ti_kcbt()    _format(296 >> 1)  // backtab key
#define ti_kend()    _format(328 >> 1)  // key end
#define ti_kent()    _format(330 >> 1)  // key enter
#define ti_kDC()     _format(382 >> 1)  // shifted delete character key
#define ti_kEND()    _format(388 >> 1)  // shifted end key
#define ti_kHOM()    _format(398 >> 1)  // shifted home key
#define ti_kIC()     _format(400 >> 1)  // shifted insert character key
#define ti_kLFT()    _format(402 >> 1)  // shifted cursor left
#define ti_kNXT()    _format(408 >> 1)  // shifted next key
#define ti_kPRV()    _format(412 >> 1)  // shifted previous key
#define ti_kRIT()    _format(420 >> 1)  // shifted cursor right
#define ti_kf11()    _format(432 >> 1)  // f11 function key
#define ti_kf12()    _format(434 >> 1)  // f12 function key

#define ti_kmous()   _format(710 >> 1)  // mouse event has occurred

// =======================================================================
