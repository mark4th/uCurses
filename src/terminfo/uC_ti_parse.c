// uC_ti_parse.c  - uCurses terminfo format string parsing
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include "uCurses.h"
#include "uC_switch.h"
#include "uC_utils.h"
#include "uC_terminfo.h"
#include "uC_utf8.h"
#include "uC_alloc.h"
#include "ti_file.h"

// -----------------------------------------------------------------------

#define ESC_SIZE (65535)    // size of escape buffer, probably overkill
#define TI_NULL  (-1)       // null terminfo file string section entry

// -----------------------------------------------------------------------
// winch support is currently disabled because i cant get it to work right

extern bool winch;

// -----------------------------------------------------------------------
// pointer to variables

ti_vars_t *ti_vars;

// -----------------------------------------------------------------------
// allocate buffers for terminfo parser

void alloc_parse(void)
{
    ti_vars = uC_alloc(uC_MEM_ZONE_DEFAULT, sizeof(*ti_vars));
    uC_ASSERT(ti_vars != NULL, "Out of Memory");

    ti_vars->esc_buff = uC_alloc(uC_MEM_ZONE_DEFAULT, ESC_SIZE);
    uC_ASSERT(ti_vars->esc_buff != NULL, "Out of Memory");
}

// -----------------------------------------------------------------------

API void uC_terminfo_flush(void)
{
    ssize_t n;

    n = write(1, ti_vars->esc_buff, ti_vars->num_esc);

    ti_vars->num_esc = 0;

    if (n < 0)
    {
        // log warning? try again?
    }
}

// -----------------------------------------------------------------------
// make sure any currently compiled but not written terminal escape
// sequences are deleted when a sigwinch is received.

void terminfo_purge(void)
{
    ti_vars->num_esc = 0;
}

// -----------------------------------------------------------------------
// write one character of escape sequence out to compilation buffer

void c_emit(char c1)
{
    // technically this is a bug, if we make it to 64k of escape
    // seauenes we cant write out the 2/3 of the current escape
    // sequence now and then the rest later...

    if (ti_vars->num_esc == ESC_SIZE)
    {
        uC_terminfo_flush();
    }

    ti_vars->esc_buff[ti_vars->num_esc++] = c1;
}

// -----------------------------------------------------------------------
// push item onto format string stack (terminfo is RPN!!)

static void fs_push(int64_t n)
{
    uC_ASSERT(ti_vars->fsp != 5, "Stack Overflow");

    ti_vars->fstack[ti_vars->fsp++] = n;
}

// -----------------------------------------------------------------------
// pop item off of format string stack

static int64_t fs_pop(void)
{
    uC_ASSERT(ti_vars->fsp != 0, "Stack Underflow");
    ti_vars->fsp--;

    return ti_vars->fstack[ti_vars->fsp];
}

// -----------------------------------------------------------------------
// scan format string to next format specifier

static char scan(void)
{
    while (*ti_vars->f_str++ != '%')
        ;
    return *ti_vars->f_str++;
}

// -----------------------------------------------------------------------
// return address of named variable

static int64_t *get_var_addr(void)
{
    int64_t *p;
    char c1;

    c1 = *ti_vars->f_str++;

    // this assumes that if it is not within the range 'a' to 'z'
    // then it is within the range 'A' to 'Z'

    p = ((c1 >= 'a') && (c1 <= 'z'))
        ? &ti_vars->atoz[c1 - 'a']
        : &ti_vars->AtoZ[c1 - 'A'];

    return p;
}

// -----------------------------------------------------------------------
// terminfo format string operators
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// as a hardcore forth programmer i cannot help but notice how all of the
// primitves in here have forth counterparts.  RPN ftw!
//
// p.s. it was either use RPN here or write some sort of recursive descent
// parser to compile escape sequences.   the original implementers chose
// to use the simplest solution and go with RPN.  IMHO so should every
// other programming language.  RDP has never been anything other than an
// ultra complexificated mountain of a solution to a freakishly miniscule
// mole hill of a problem.

// -----------------------------------------------------------------------
// format = %%

static void _percent(void)
{
    c_emit((uint8_t)'%');
}

// -----------------------------------------------------------------------
// format = %&   bitwise and

static void _and(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 & n2);
}

// -----------------------------------------------------------------------
// format = %A  logical and

static void _andl(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 && n2);
}

// -----------------------------------------------------------------------
// format = %|    bitwise or

static void _or(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 | n2);
}

// -----------------------------------------------------------------------
// format = %O    logical or

static void _orl(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 || n2);
}

// -----------------------------------------------------------------------
// format = %~     not

static void _not  (void)
{
    int64_t n1;

    n1 = fs_pop();

    fs_push(~n1);
}

// -----------------------------------------------------------------------
// format = %!   logical not

static void _notl(void)
{
    int64_t n1;

    n1 = fs_pop();

    n1 = (n1) ? 0 : 1;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %^

static void _xor  (void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    fs_push(n1 ^ n2);
}

// -----------------------------------------------------------------------
// format = %+

static void _plus(void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    fs_push(n1 + n2);
}

// -----------------------------------------------------------------------
// format = %-

static void _minus(void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    fs_push(n1 - n2);
}

// -----------------------------------------------------------------------
// format = %*

static void _star(void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    fs_push(n1 * n2);
}

// -----------------------------------------------------------------------
// format = %/

static void _div (void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    (n2 != 0)
        ? fs_push(n1 / n2)
        : fs_push(0);
}

// -----------------------------------------------------------------------
// format = %m

static void _mod(void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    (n2 != 0)
        ? fs_push(n1 % n2)
        : fs_push(0);
}

// -----------------------------------------------------------------------
//  format = %=

static void _equals(void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    n1 = (n2 == n1) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %>

static void _greater(void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    n1 = (n1 > n2) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %<

static void _less(void)
{
    int64_t n1, n2;

    n2 = fs_pop();
    n1 = fs_pop();

    n1 = (n1 < n2) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %'

static void _tick(void)
{
    char c1;

    c1 = *ti_vars->f_str;
    fs_push(c1);

    ti_vars->f_str += 2;
}

// -----------------------------------------------------------------------
// format = %i

// for ansi terminals the first two parameters are ONE based not zero
// based so for example, x/y coordinates start at 1 not 0

static void _i(void)
{
    ti_vars->params[0]++;  // increment first two parameters
    ti_vars->params[1]++;  // for ansi terminals
}

// -----------------------------------------------------------------------
// format = %s

static void _s(void)
{
    char *s;
    char c1;

    s = (char *)fs_pop();

    if (s != NULL)
    {
        while ((c1 = *s++))
        {
            c_emit(c1);
        }
    }
}

// -----------------------------------------------------------------------
// format = %l

static void _l(void)
{
    uint8_t *s;
    int16_t len;

    s = (uint8_t *)fs_pop();

    if (s != NULL)
    {
        len = uC_utf8_strlen(s);
        fs_push(len);
    }
}

// -----------------------------------------------------------------------
// format = %P     store top item of stack into specified variable

static void _P(void)
{
    int64_t *s;

    s = get_var_addr();

    *s = fs_pop();
}

// -----------------------------------------------------------------------
// format = &g      push specified variables value onto stack

static void _g(void)
{
    int64_t *s;

    s = get_var_addr();

    fs_push(*s);
}

// -----------------------------------------------------------------------
// format = %{   parse number between { and } in decimal put its value on
// the stack

static void _brace(void)
{
    int64_t n1;
    char c1;

    n1 = 0;

    while ((c1 = *ti_vars->f_str++) != '}')
    {
        n1 *= 10;
        n1 += (c1 - '0');
    }

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %t

static void _t(void)
{
    char c1;

    int64_t f1 = fs_pop();  // if this is non 0 we dont do anything

    if (f1 != 0)            // if it is 0 we skip past then part
    {
        return;
    }

    // false condition exists, scan to the %e or the %;

    for (;;)
    {
        c1 = scan();

        // break if we are at the else or endif....
        if ((c1 == 'e') || (c1 == ';'))
        {
            break;
        }
    }
}

// -----------------------------------------------------------------------
// format = %e

static void _e(void)
{
    char c1;

    do
    {
        c1 = scan();
    } while (c1 != ';');
}

// -----------------------------------------------------------------------
// format = %d

static void _d(void)
{
    int64_t n1;
    n1 = fs_pop();
    ti_vars->digits--;

    const char widths[3][6] =
    {
        "%"   PRIu64,
        "%02" PRIu64,
        "%03" PRIu64
    };

    // this is a bug, if we are mid escape sequence we cant flush
    // a partial now then the rest later

    if ((ESC_SIZE - ti_vars->num_esc) < 6)
    {
        uC_terminfo_flush();
    }

    ti_vars->num_esc += snprintf((char *)&ti_vars->esc_buff[ti_vars->num_esc],
        4, widths[ti_vars->digits], n1);
}

// -----------------------------------------------------------------------
// format = %c

static void _c(void)
{
    int64_t c1;

    c1 = fs_pop();
    c_emit(c1);
}

// -----------------------------------------------------------------------
// format = %p

static void _p(void)
{
    uint8_t c1;

    // get parameter number from format string

    c1 = *ti_vars->f_str++;
    c1 &= 0x0f;
    c1--;

    fs_push(ti_vars->params[c1]);
}

// -----------------------------------------------------------------------

static char next_c(void)
{
    char c1;

    ti_vars->digits = 1;
    c1 = *ti_vars->f_str++;

    if ((c1 == '2') || (c1 == '3'))
    {
        ti_vars->digits = (c1 & 0x0f);
        c1 = *ti_vars->f_str++;
    }

    return c1;
}

// -----------------------------------------------------------------------
// terminfo format string % codes

static const uC_switch_t p_codes[] =
{
    { '%', &_percent }, { 'p', &_p      }, { 'd', &_d       },
    { 'c', &_c       }, { 'i', &_i      }, { 's', &_s       },
    { 'l', &_l       }, { 'A', &_andl   }, { '&', &_and     },
    { 'O', &_orl     }, { '|', &_or     }, { '!', &_notl    },
    { '~', &_not     }, { '^', &_xor    }, { '+', &_plus    },
    { '-', &_minus   }, { '*', &_star   }, { '/', &_div     },
    { 'm', &_mod     }, { '=', &_equals }, { '>', &_greater },
    { '<', &_less    }, { 0x27, &_tick  }, { '{', &_brace   },
    { 'P', &_P       }, { 'g', &_g      }, { '?', &uC_noop  },
    { 't', &_t       }, { 'e', &_e      }, { ';', &uC_noop  },
};

#define PCOUNT (sizeof(p_codes) / sizeof(p_codes[0]))

// -----------------------------------------------------------------------
// process a % command char from format string c1 = char following %

static inline void specifier(char c1)
{
    uC_switch(p_codes, PCOUNT, c1);
}

// -----------------------------------------------------------------------
// parse format string at specified address

API void uC_parse_format(const uint8_t *f)
{
    char c1;

    ti_vars->f_str = f;

    while (*ti_vars->f_str != '\0')
    {
        c1 = *ti_vars->f_str++;

        // if the following code is confusing or unreadable to you then
        // you need to rethink your career choice kthxbai

        (c1 == '%')
            ? specifier(next_c())
            : c_emit(c1);
    }
}

// -----------------------------------------------------------------------
// parse a terminfo format string from the terminfo files strings section

void uC_format(int16_t i)
{
    i = ti_vars->ti_file.ti_strings[i];

    // it is not an error for a format string to be blank
    if (i != TI_NULL)
    {
        uC_parse_format((uint8_t *)&ti_vars->ti_file.ti_table[i]);
    }
}

// =======================================================================
