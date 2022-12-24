// parse.c  - uCurses terminfo format string parsing
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

// -----------------------------------------------------------------------

#define ESC_SIZE (65535)

extern ti_file_t *ti_file;

// -----------------------------------------------------------------------

typedef struct
{
    int8_t digits;          // number of digits for %d (2 or 3)
    int8_t fsp;             // stack pointer for ...
    int64_t fstack[5];      // format string stack
    int64_t atoz[26];       // named format string variables
    int64_t AtoZ[26];
} private_t;

// -----------------------------------------------------------------------
// pointerd to variables

ti_parse_t *uC_ti_parse;
static private_t  *vars;

// -----------------------------------------------------------------------
// allocate buffers for terminfo parser

void alloc_parse(void)
{
    bool result;

    uC_ti_parse = calloc(1, sizeof(*uC_ti_parse));
    result      = (uC_ti_parse != NULL);

    // allocate 64k for compiled escape sequences
    if (result)
    {
        uC_ti_parse->esc_buff = calloc(1, ESC_SIZE);
        result = (uC_ti_parse->esc_buff != NULL);
    }

    if (result)
    {
        vars = calloc(1, sizeof(*vars));
        result = (vars != NULL);
    }

    if (!result)
    {
        printf("uCurses: Out of Memory allocating buffers\r\n");
        exit(1);
    }
}

// -----------------------------------------------------------------------

void free_parse(void)
{
    free(uC_ti_parse->esc_buff);
    free(uC_ti_parse);
}

// -----------------------------------------------------------------------

API void uC_terminfo_flush(void)
{
    ssize_t n;

    n = write(1, uC_ti_parse->esc_buff, uC_ti_parse->num_esc);
    uC_ti_parse->num_esc = 0;

    if (n < 0)
    {
        // log warning? try again?
    }
}

// -----------------------------------------------------------------------
// write one character of escape sequence out to compilation buffer

void c_emit(char c1)
{
    // technically this is a bug, if we make it to 64k of escape
    // seauenes we cant write out the 2/3 of the current escape
    // sequence now and then the rest later...

    if (uC_ti_parse->num_esc == ESC_SIZE)
    {
        uC_terminfo_flush();
    }

    uC_ti_parse->esc_buff[uC_ti_parse->num_esc++] = c1;
}

// -----------------------------------------------------------------------
// push item onto format string stack (terminfo is RPN!!)

static void fs_push(int64_t n)
{
    uC_ASSERT(vars->fsp != 5, "Stack Overflow");

    vars->fstack[vars->fsp++] = n;
}

// -----------------------------------------------------------------------
// pop item off of format string stack

static int64_t fs_pop(void)
{
    uC_ASSERT(vars->fsp != 0, "Stack Underflow");
    vars->fsp--;

    return vars->fstack[vars->fsp];
}

// -----------------------------------------------------------------------
// terminfo format string operators
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// as a hardcore forth programmer i cannot help but notice how all of the
// primitves in here have forth counterparts.  RPN ftw!
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// format = %%

static void _percent(void)
{
    c_emit('%');
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

static void _tilde(void)
{
    int64_t n1;

    n1 = fs_pop();

    fs_push(~n1);
}

// -----------------------------------------------------------------------
// format = %!   logical not

static void _bang(void)
{
    int64_t n1;

    n1 = fs_pop();

    fs_push(!n1);
}

// -----------------------------------------------------------------------
// format = %^

static void _caret(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 ^ n2);
}

// -----------------------------------------------------------------------
// format = %+

static void _plus(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 + n2);
}

// -----------------------------------------------------------------------
// format = %-

static void _minus(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n2 - n1);
}

// -----------------------------------------------------------------------
// format = %*

static void _star(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n2 * n1);
}

// -----------------------------------------------------------------------
// format = %/

static void _slash(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    (n1 != 0)
        ? fs_push(n2 / n1)
        : fs_push(0);
}

// -----------------------------------------------------------------------
// format = %m

static void _mod(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    (n1 != 0)
        ? fs_push(n2 % n1)
        : fs_push(0);
}

// -----------------------------------------------------------------------
//  format = %=

static void _equals(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    n1 = (n2 == n1) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %>

static void _greater(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    n1 = (n2 > n1) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %<

static void _less(void)
{
    int64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    n1 = (n2 < n1) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %'

static void _tick(void)
{
    char c1;

    c1 = *uC_ti_parse->f_str;
    fs_push(c1);

    uC_ti_parse->f_str += 2;
}

// -----------------------------------------------------------------------
// format = %i

// for ansi terminals the first two parameters are ONE based not zero
// based so x/y oordinates start at 1 not 0

static void _i(void)
{
    uC_ti_parse->params[0]++;  // increment first two parameters
    uC_ti_parse->params[1]++;  // for ansi terminals
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
    char *s;
    int16_t len;

    s = (char *)fs_pop();

    if (s != NULL)
    {
        len = uC_utf8_strlen(s);
        fs_push(len);
    }
}

// -----------------------------------------------------------------------
// return address of named variable

static int64_t *get_var_addr(void)
{
    int64_t *p;
    char c1;

    c1 = *uC_ti_parse->f_str++;

    // this assumes that if it is not within the range 'a' to 'z'
    // then it is within the range 'A' to 'Z'

    p = ((c1 >= 'a') && (c1 <= 'z'))
        ? &vars->atoz[c1 - 'a']
        : &vars->AtoZ[c1 - 'A'];

    return p;
}

// -----------------------------------------------------------------------
// format = %P

static void _P(void) // store top item of stack into specified variable
{
    int64_t *s;

    s = get_var_addr();

    *s = fs_pop();
}

// -----------------------------------------------------------------------
// format = &g

static void _g(void) // push specified variables value onto stack
{
    int64_t *s;

    s = get_var_addr();

    fs_push(*s);
}

// -----------------------------------------------------------------------
// format = %{

static void _brace(void) // parse number between { and } in decimal
{                        // put its value on the stack
    int64_t n1;
    char c1;

    n1 = 0;

    while ((c1 = *uC_ti_parse->f_str++) != '}')
    {
        n1 *= 10;
        n1 += (c1 - '0');
    }

    fs_push(n1);
}

// -----------------------------------------------------------------------
// skip forward in format string to next % command

static void to_cmd(void)
{
    while (*uC_ti_parse->f_str++ != '%')
        ;
}

// -----------------------------------------------------------------------

void scan_to_endif(void)
{
    char c1;
    int8_t nest;            // not sure if any terminfo has nested %?

    nest = 0;

    for (;;)
    {
        // scan format string for next % char
        to_cmd();
        c1 = *uC_ti_parse->f_str++;

        // if we are nesting if's count depth
        if (c1 == '?') { nest++; }

        // if we are at the else or endif....
        if ((c1 == 'e') || (c1 == ';'))
        {
            if (nest == 0) // break out of loop if at else or endif
            {              // and we have scanned past all nested %?
                break;
            }
            // we are within a nested %? -
            // must scan t0 %; before we break
            else if (c1 == ';')
            {
                nest--;
            }
        }
    }
}

// -----------------------------------------------------------------------
// format = %t

static void _t(void)
{
    int64_t f1 = fs_pop();  // if this is non 0 we dont do anything

    if (f1 == 0)            // if it is 0 we skip past then part
    {
        scan_to_endif();
    }
}

// -----------------------------------------------------------------------
// format = %e

static void _e(void)
{
    char c1;
    int8_t nest = 0;

    for (;;)
    {
        to_cmd();

        c1 = *uC_ti_parse->f_str++;

        if (c1 == '?')
        {
            nest++;
        }
        else if (c1 == ';')
        {
            if (nest == 0) { break; }
            nest--;
        }
    }
}

// -----------------------------------------------------------------------
// format = %d

static void _d(void)
{
    int64_t n1;
    n1 = fs_pop();
    vars->digits--;

    const char widths[3][6] =
    {
        "%"   PRIu64,
        "%02" PRIu64,
        "%03" PRIu64
    };

    // this is a bug, if we are mid escape sequence we cant flush
    // a partial now then the rest later

    if ((ESC_SIZE - uC_ti_parse->num_esc) < 6)
    {
        uC_terminfo_flush();
    }

    uC_ti_parse->num_esc += snprintf(&uC_ti_parse->esc_buff[uC_ti_parse->num_esc],
        4, widths[vars->digits], n1);
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
    int8_t c1;

    // get parameter number from format string

    c1 = *uC_ti_parse->f_str++;
    c1 &= 0x0f;
    c1--;

    fs_push(uC_ti_parse->params[c1]);
}

// -----------------------------------------------------------------------

static char next_c(void)
{
    char c1;

    vars->digits = 1;
    c1 = *uC_ti_parse->f_str++;

    if ((c1 == '2') || (c1 == '3'))
    {
        vars->digits = (c1 & 0x0f);
        // this should be a d ... should i test that?
        c1 = *uC_ti_parse->f_str++;
    }
    return c1;
}

// -----------------------------------------------------------------------
// terminfo format string % codes

static const switch_t p_codes[] =
{
    { '%', &_percent }, { 'p', &_p      }, { 'd', &_d       },
    { 'c', &_c       }, { 'i', &_i      }, { 's', &_s       },
    { 'l', &_l       }, { 'A', &_andl   }, { '&', &_and     },
    { 'O', &_orl     }, { '|', &_or     }, { '!', &_bang    },
    { '~', &_tilde   }, { '^', &_caret  }, { '+', &_plus    },
    { '-', &_minus   }, { '*', &_star   }, { '/', &_slash   },
    { 'm', &_mod     }, { '=', &_equals }, { '>', &_greater },
    { '<', &_less    }, { 0x27, &_tick  }, { '{', &_brace   },
    { 'P', &_P       }, { 'g', &_g      }, { '?', &uC_noop  },
    { 't', &_t       }, { 'e', &_e      }, { ';', &uC_noop  },
};

#define PCOUNT (sizeof(p_codes) / sizeof(p_codes[0]))

// -----------------------------------------------------------------------
// process a % command char from format string c1 = char following %

static void cmd(char c1)
{
    re_switch(p_codes, PCOUNT, c1);
}

// -----------------------------------------------------------------------
// parse format string at specified address

API void uC_parse_format(const char *f)
{
    char c1;

    uC_ti_parse->f_str = f;

    while (*uC_ti_parse->f_str)
    {
        c1 = *uC_ti_parse->f_str++;

        (c1 == '%')
            ? cmd(next_c())
            : c_emit(c1);
    }
}

// -----------------------------------------------------------------------
// parse a terminfo format string from the terminfo files strings section

void uC_format(int16_t i)
{
     i = ti_file->ti_strings[i];

    // it is not an error for a format string to be blank
    if (i != -1)
    {
        uC_parse_format(&ti_file->ti_table[i]);
    }
}

// =======================================================================
