// uCurses.h
// -----------------------------------------------------------------------

#pragma once

// -----------------------------------------------------------------------

#include <inttypes.h>

// -----------------------------------------------------------------------

#define MAX_PARAM 40

// -----------------------------------------------------------------------

typedef void (*fp_t)(void *x);

// -----------------------------------------------------------------------

extern uint64_t params[MAX_PARAM];

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

// -----------------------------------------------------------------------

void c2$(uint8_t c1);
void _format(uint16_t i);
void uCurses_init(void);
void set_attribs(void);
void _send$(void *unused);

extern fp_t send$;

// -----------------------------------------------------------------------
// from utils.c

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

// =======================================================================
