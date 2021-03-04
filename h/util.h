// util.h
// -----------------------------------------------------------------------

    #pragma once

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

// =======================================================================
