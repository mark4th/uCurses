// parse.c  - uCurses format string parsing                 (c) 2019 MIMIV
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "h/tui.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------

typedef void (*opt_t)(void);
typedef struct
{
  uint8_t option;
  opt_t vector;
} switch_t;

// -----------------------------------------------------------------------

static void noop(void){;}

// -----------------------------------------------------------------------

uint8_t *str_buff;           // format string compilation output buffer
uint16_t nb;                // max of 64k of compiled escape sequences

uint8_t fsp;                // stack pointer for ...
uint64_t fstack[5];         // format string stack

uint8_t *f_str;             // pointer to next character of format string
uint8_t digits;             // number of digits for %d (2 or 3)

uint64_t params[MAX_PARAM]; // format string parametesr

uint64_t atoz[26];          // named format string variables
uint64_t AtoZ[26];

uint8_t compile = 0;        // set to 1 to compile up to 64K of escapes

fp_t *send_str;             // write all format strings to

// -----------------------------------------------------------------------
// addresses within memory mapped terminfo file

extern uint8_t *ti_table;   // array of offsets within following
extern uint16_t *ti_strings;

// -----------------------------------------------------------------------
// write all compiled format strings to stdout

void _send_str(void *unused)
{
  int n;

  str_buff[nb] = '\0';

 // TODO: hard coded out file is wrong
 n =  write(1, &str_buff[0], nb);

 if(n < 0)
 {
   // TODO: log warning?
 }

  nb = 0;
}

// -----------------------------------------------------------------------
// debug

void print_format(uint8_t *p)
{
  while(*p)
  {
    (*p < 0x20)
      ? printf(" %02x ", *p)
      : printf("%c", *p);
    p++;
  }
  printf("\r\n");
}

// -----------------------------------------------------------------------
// translate charcter to alt charset

uint8_t to_acsc(uint8_t c1)
{
  uint16_t offset;
  uint8_t *t;

  offset = ti_strings[292];

  t = &ti_table[offset];

  for(; *t != c1; t += 2)
  {
    // how exactly am i SUPPOSED to terminate this? is the table always
    // the same length?
    c1 = t[1];
  }

  return c1;
}

// -----------------------------------------------------------------------
// push item onto format string stack (terminfo is RPN!!)

static void fs_push(uint64_t n)
{
  if(5 != fsp)
  {
    fstack[fsp++] = n;
    return;
  }
  // methinks this might could be an internal error
  // abort "uCurses format string stck overflow"
}

// -----------------------------------------------------------------------
// pop item off of format string stack

static uint64_t fs_pop(void)
{
  if(fsp)
  {
    return fstack[--fsp];
  }
  // methinks this might could be an internal error
  // abort "uCurses format string stck underflow"
  // TODO look at at above comment
  return 0;
}

// -----------------------------------------------------------------------
// write one character of escape sequence out to compilation buffer

void c_emit(uint8_t c1)
{
  str_buff[nb++] = c1;
}

// -----------------------------------------------------------------------
// terminfo format string operators
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// output a '%' character

static void _percent(void)
{
  c_emit('%');
}

// -----------------------------------------------------------------------
// logical and of two format string stack items

static void _and(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n1 & n2);
}

// -----------------------------------------------------------------------

static void _or(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n1 | n2);
}

// -----------------------------------------------------------------------

static void _bang(void)
{
  uint64_t n1;

  n1 = fs_pop();
  n1 = (n1 == 0) ? 1 : 0;

  fs_push(n1);
}

// -----------------------------------------------------------------------

static void _tilde(void)
{
  uint64_t n1;

  n1 = fs_pop();

  fs_push(~n1);
}

// -----------------------------------------------------------------------

static void _caret(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n1 ^ n2);
}

// -----------------------------------------------------------------------

static void _plus(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n1 + n2);
}

// -----------------------------------------------------------------------

static void _minus(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n2 - n1);
}

// -----------------------------------------------------------------------

static void _star(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n2 - n1);
}

// -----------------------------------------------------------------------

static void _slash(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n2 / n1);
}

// -----------------------------------------------------------------------

static void _m(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n2 % n1);
}

// -----------------------------------------------------------------------

static void _equals(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  n1 = (n2 == n1) ? 1 : 0;

  fs_push(n1);
}

// -----------------------------------------------------------------------

static void _greater(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  n1 = (n2 > n1) ? 1 : 0;

  fs_push(n1);
}

// -----------------------------------------------------------------------

static void _less(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  n1 = (n2 < n1) ? 1 : 0;

  fs_push(n1);
}

// -----------------------------------------------------------------------
// compile ascii char 'x'

static void _tick(void)
{
  uint8_t c1;

  c1 = *f_str++;
  c_emit(c1);
  f_str++;
}

// -----------------------------------------------------------------------
// increment first two parameters for ansi terminals

static void _i(void)
{
  params[0]++;
  params[1]++;
}

// -----------------------------------------------------------------------

static void _s(void)
{
  uint8_t *s;
  uint8_t c1;

  s = (uint8_t *)fs_pop();

  while((c1 = *s++))
  {
    c_emit(c1);
  }
}

// -----------------------------------------------------------------------

static void _l(void)
{
  uint8_t *s;

  s = (uint8_t *)fs_pop();

  fs_push(strlen((const char *)s));
}

// -----------------------------------------------------------------------

static uint64_t *q_atoz(void)
{
  uint64_t *p;
  uint8_t c1;

  c1 = *f_str++;

  p = ((c1 >= 'a') && (c1 <= 'z'))
    ? &atoz[c1 - 'a']
    : &AtoZ[c1 - 'A'];

  return p;
}

// -----------------------------------------------------------------------

static void _P(void)
{
  uint64_t *s;

  s = q_atoz();

  *s = fs_pop();
}

// -----------------------------------------------------------------------

static void _g(void)
{
  uint64_t *s;

  s = q_atoz();

  fs_push(*s);
}

// -----------------------------------------------------------------------

static void _brace(void)
{
  uint64_t n1;
  uint8_t c1;

  n1 = 0;

  while ('}' != *f_str)
  {
     c1 = *f_str++;
     n1 *= 10;
     n1 += (c1 - '0');
  }

  f_str++;
  fs_push(n1);
}

// -----------------------------------------------------------------------
// skip forward in format string to nextg % command

static void to_cmd(void)
{
  while('%' != *f_str++)
    ;
}

// -----------------------------------------------------------------------
// too much if/and/but loop nesting

static void _t(void)
{
  uint64_t f1;
  uint8_t c1;
  uint8_t nest;             // not sure if any terminfo has nested %?

  nest = 0;

  f1 = fs_pop();            // if this is non 0 we dont do anything

  if(0 == f1)               // if it is 0 we skip past then part
  {
    for(;;)
    {
      to_cmd();             // scan format string for next % char
      c1 = *f_str++;

      if('?' == c1)         // if we are nesting if's count depth
      {
        nest++;
      }
      // if we are at the else or endif....
      if(('e' == c1) || (';' == c1))
      {
        if(0 == nest)       // break out of loop if at else or endif
        {                   // and we have scanned past all nested %?
          break;
        }
        // we are within a nested %? - must scan t0 %; before we break
        else if(';' == c1)
        {
          nest--;
        }
      }
    }
  }
}

// -----------------------------------------------------------------------

static void _e(void)
{
  uint8_t c1;
  uint8_t nest=0;

  for(;;)
  {
    to_cmd();

    c1 = *f_str++;

    if('?' == c1)
    {
      nest++;
    }
    if(';' == c1)
    {
      if(0 == nest)
      {
        break;
      }
      nest--;
    }
  }
}

// -----------------------------------------------------------------------

static void _d(void)
{
  uint64_t n1, n2;

  uint8_t s[20];
  uint8_t *p;

  p = &s[0];

  n1 = fs_pop();

  switch(digits)
  {
    case 2:
      n2 = snprintf((char *)s, 2, "%ld", n1);
      break;
    case 3:
      n2 = snprintf((char *)s, 3, "%ld", n1);
      break;
    default :
      n2 = snprintf((char *)s, 4, "%ld", n1);
  }

  s[n2]='\0';

  strncat((char *)&str_buff[0], (char *)s, n2);
  nb += n2;
}

// -----------------------------------------------------------------------

static void _c(void)
{
  uint64_t c1;

  c1 = fs_pop();
  c_emit(c1);
}

// -----------------------------------------------------------------------

static void _p(void)
{
  uint8_t c1;

  c1 = *f_str++;
  c1 &= 0x0f;
  c1--;
  c1 = params[c1];
  fs_push(c1);
}

// -----------------------------------------------------------------------

static uint8_t next_c(void)
{
  uint8_t c1;

  c1 = *f_str++;

  if(('2' == c1) || ('3' == c1))
  {
    digits = (c1 & 0x0f);
    // this should be a d
    c1 = *f_str++;
  }
  return c1;
}

// -----------------------------------------------------------------------
// terminfo format string % codes

const switch_t p_codes[] =
{
  { '%', &_percent }, { 'p', &_p },     { 'd', &_d },     { 'c', &_c },
  { 'i', &_i },       { 's', &_s },     { 'l', &_l },     { 'A', &_and },
  { '&', &_and },     { 'O', &_or },    { '|', &_or },    { '!', &_bang },
  { '~', &_tilde },   { '^', &_caret }, { '+', &_plus },  { '-', &_minus },
  { '*', &_star },    { '/', &_slash }, { 'm', &_m },     { '=', &_equals },
  { '>', &_greater }, { '<', &_less },  { 0x27, &_tick }, { '{', &_brace },
  { 'P', &_P },       { 'g', &_g },     { 't', &_t },     { 'e', &_e },
  { '?', &noop },     { ';', &noop },
};

// -----------------------------------------------------------------------

static void command(void)
{
  uint8_t c1;
  const switch_t *s = &p_codes[0];

  int n = sizeof(p_codes) / sizeof(p_codes[0]);

  c1 = next_c();

  while(n--)
  {
    if(c1 == s->option)
    {
      (s->vector)();
      break;
    }
    s++;
  }
}

// -----------------------------------------------------------------------

void _format(uint16_t i)
{
  uint8_t c1;

  i = ti_strings[i];

  // it is not an error for a format string to be blank
  if(0xffff == i)
  {
    return;
  }

  f_str = &ti_table[i];

  while((c1 = *f_str++))
  {
    ('%' == c1)
      ? command()
      : c_emit(c1);
  }

  // we either post the compiled escape sequence now or we stack up
  // up to 64k of them in str_buff[] and output them all later

  (*send_str)(0);         // this might be a do nothng
}

// =======================================================================
