// parse.c  - uCurses format string parsing                 (c) 2019 MIMIV
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>

#include "h/uCurses.h"
#include "h/tui.h"
#include "h/uCurses_types.h"

// -----------------------------------------------------------------------

// I know using $ in identifies is not portable, im just a heretic.
// and this is not production code, its just a quick and dirty example
// of how someone might do this if they needed to

// -----------------------------------------------------------------------

uint8_t *buffer;            // format string compilation output buffer
uint16_t bix;                // max of 64k of compiled escape sequences

uint8_t fsp;                // stack pointer for ...
uint64_t fstack[5];         // format string stack

uint8_t *f_str;                // pointer to next character of format string
uint8_t digits;             // number of digits for %d (2 or 3)

uint64_t params[MAX_PARAM]; // format string parametesr

uint64_t atoz[26];          // named format string variables
uint64_t AtoZ[26];

uint8_t compile = 0;        // set to 1 to compile up to 64K of escapes

fp_t send_stdout;                 // write all format strings to

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// write all compiled format strings to stdout

void _send_stdout(void *unused)
{
  write(1, &buffer[0], bix);
  bix = 0;
}

// -----------------------------------------------------------------------
// addresses within memory mapped terminfo file

extern uint8_t *ti_table;   // array of offsets within following
extern uint16_t *ti_strings;

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

uint8_t t_acsc(uint8_t c1)
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

void c2w(uint8_t c1)
{
  buffer[bix++] = c1;
  buffer[bix] = '\0';
}

// -----------------------------------------------------------------------
// terminfo format string operators
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// output a '%' character

static inline void _percent(void)
{
  c2w('%');
}

// -----------------------------------------------------------------------
// logical and of two format string stack items

static inline void _and(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n1 & n2);
}

// -----------------------------------------------------------------------

static inline void _or(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n1 | n2);
}

// -----------------------------------------------------------------------
#ifdef DAS_NEVER_DEFINED
static inline void _bang(void)
{
  uint64_t n1;

  n1 = fs_pop();
  n1 = (n1 == 0) ? 1 : 0;

  fs_push(n1);
}
#endif // DAS_NEVER_DEFINED

// -----------------------------------------------------------------------

#ifdef DAS_NEVER_DEFINED
static inline void _tilde(void)
{
  uint64_t n1;

  n1 = fs_pop();

  fs_push(~n1);
}
#endif // DAS_NEVER_DEFINED

// -----------------------------------------------------------------------

static inline void _caret(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n1 ^ n2);
}

// -----------------------------------------------------------------------

static inline void _plus(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n1 + n2);
}

// -----------------------------------------------------------------------

static inline void _minus(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n2 - n1);
}

// -----------------------------------------------------------------------

static inline void _star(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n2 - n1);
}

// -----------------------------------------------------------------------

static inline void _slash(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n2 / n1);
}

// -----------------------------------------------------------------------

static inline void _m(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  fs_push(n2 % n1);
}

// -----------------------------------------------------------------------

static inline void _equals(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  n1 = (n2 == n1) ? 1 : 0;

  fs_push(n1);
}

// -----------------------------------------------------------------------

static inline void _greater(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  n1 = (n2 > n1) ? 1 : 0;

  fs_push(n1);
}

// -----------------------------------------------------------------------

static inline void _less(void)
{
  uint64_t n1, n2;

  n1 = fs_pop();
  n2 = fs_pop();

  n1 = (n2 < n1) ? 1 : 0;

  fs_push(n1);
}

// -----------------------------------------------------------------------
// compile ascii char 'x'

static inline void _tick(void)
{
  uint8_t c1;

  c1 = *f_str++;
  c2w(c1);
  f_str++;
}

// -----------------------------------------------------------------------
// increment first two parameters for ansi terminals

static inline void _i(void)
{
  params[0]++;
  params[1]++;
}

// -----------------------------------------------------------------------

#ifdef DAS_NEVER_DEFINED
static inline void _s(void)
{
  uint8_t *s;
  uint8_t c1;

  s = (uint8_t *)fs_pop();

  while((c1 = *s++))
  {
    c2$(c1);
  }
}
#endif // DAS_NEVER_DEFINED

// -----------------------------------------------------------------------

#ifdef DAS_NEVER_DEFINED
static inline void _l(void)
{
  uint8_t *s;

  s = (uint8_t *)fs_pop();

  fs_push(strlen(cstr(s)));
}
#endif //DAS_NEVER_DEFINED
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

static inline void _P(void)
{
  uint64_t *s;

  s = q_atoz();

  *s = fs_pop();
}

// -----------------------------------------------------------------------

static inline void _g(void)
{
  uint64_t *s;

  s = q_atoz();

  fs_push(*s);
}

// -----------------------------------------------------------------------

static inline void _brace(void)
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

static inline void to_cmd(void)
{
  while('%' != *f_str++)
    ;
}

// -----------------------------------------------------------------------
// too much if/and/but loop nesting

static inline void _t(void)
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

static inline void _e(void)
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

static inline void _d(void)
{
  uint64_t n1, n2;

  uint8_t s[20];
  uint8_t *p;

  p = &s[0];

  n1 = fs_pop();
  n2 = snprintf((str)s, 4, "%ld", n1);

  s[n2]='\0';

  strncat((str)&buffer[0], (str)s, n2);
  bix += n2;
}

// -----------------------------------------------------------------------

static inline void _c(void)
{
  uint64_t c1;

  c1 = fs_pop();
  c2w(c1);
}

// -----------------------------------------------------------------------

static inline void _p(void)
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
    digits = (*f_str++ & 0x0f);
    c1 = *f_str++;
  }
  return c1;
}

// -----------------------------------------------------------------------

static inline void command(void)
{
  uint8_t c1;

  c1 = next_c();

  switch(c1)
  {
    case '%': _percent();  break;
    case 'p': _p();        break;
    case 'd': _d();        break;
    case 'c': _c();        break;
    case 'i': _i();        break;
    case 'A': // technically this is wrong
    case '&': _and();      break;
    case 'O': // so is this :)
    case '|': _or();       break;
    case '^': _caret();    break;
    case '+': _plus();     break;
    case '-': _minus();    break;
    case '*': _star();     break;
    case '/': _slash();    break;
    case 'm': _m();        break;
    case '=': _equals();   break;
    case '>': _greater();  break;
    case '<': _less();     break;
    case 0x27: _tick();    break;
    case '{': _brace();    break;
    case 'P': _P();        break;
    case 'g': _g();        break;
    case 't': _t();        break;
    case 'e': _e();        break;
    case '?':
    case ';':              break;
  }
}

// -----------------------------------------------------------------------

void _format(uint16_t i)
{
  //uint32_t n; DAS NOT USED
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
      : c2w(c1);
  }

  // we either post the compiled escape sequence now or we stack up
  // up to 64k of them in buffer[] and output them all later

  (send_stdout)(NULL);            // this might be a do nothng
}

// =======================================================================
