// parse.c  - uCurses terminfo format string parsing
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------
// for format string % commands that dont actually need to do anything

static void noop(void) { ; }

// -----------------------------------------------------------------------

char *esc_buff;            // format string compilation output buffer
uint16_t num_esc;           // max of 64k of compiled escape seq bytes
int64_t params[MAX_PARAM]; // format string parametesr

static int8_t fsp;        // stack pointer for ...
static int64_t fstack[5]; // format string stack

const char *f_str;    // pointer to next character of format string
static int8_t digits; // number of digits for %d (2 or 3)

static int64_t atoz[26]; // named format string variables
static int64_t AtoZ[26];

// -----------------------------------------------------------------------
// addresses within memory mapped terminfo file

extern char *ti_table; // array of offsets within following
extern int16_t *ti_strings;

// -----------------------------------------------------------------------
// debug

// FILE *log_fp;
//
// void log_dump(void)
// {
//     int16_t i;
//     char *p = esc_buff;
//
//     for(i = 0; i < num_esc; i++)
//     {
//         if(*p == 0x1b)
//         {
//             fprintf(log_fp, "\n");
//         }
//         (*p <= 0x1f)
//             ? fprintf(log_fp, "。%02x", (uint8_t)*p)
//             : fprintf(log_fp, "%c", *p);
//         p++;
//     }
//     fprintf(log_fp, "\n\n");
// }

// -----------------------------------------------------------------------

void flush(void)
{
    ssize_t n;

    // log_dump();

    n = write(1, esc_buff, num_esc);
    num_esc = 0;

    if(n < 0)
    {
        // log warning? try again?
    }
}

// -----------------------------------------------------------------------
// write one character of escape sequence out to compilation buffer

void c_emit(char c1)
{
    esc_buff[num_esc++] = c1;

    // technically this is a bug, if we make it to 64k of escape
    // seauenes we cant write out the 2/3 of the current escape
    // sequence now and then the rest later...

    if(num_esc == 0xffff)
    {
        flush();
    }
}

// -----------------------------------------------------------------------
// push item onto format string stack (terminfo is RPN!!)

static void fs_push(int64_t n)
{
    if(fsp != 5)
    {
        fstack[fsp++] = n;
        return;
    }
    // methinks this might could be an internal error
    // abort" uCurses format string stack overflow"
}

// -----------------------------------------------------------------------
// pop item off of format string stack

static int64_t fs_pop(void)
{
    int16_t rv;

    rv = (fsp != 0) //
             ? fstack[--fsp]
             : 0; // also an internal error?

    return rv;
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

static void _percent(void) { c_emit('%'); }

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

    n1 = (fs_pop() != 0) ? 1 : 0;
    n2 = (fs_pop() != 0) ? 1 : 0;

    fs_push(n1 & n2);
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

    n1 = (fs_pop() != 0) ? 1 : 0;
    n2 = (fs_pop() != 0) ? 1 : 0;

    fs_push(n1 | n2);
}

// -----------------------------------------------------------------------
// format = %!

static void _bang(void)
{
    int64_t n1;

    n1 = fs_pop();
    n1 = (0 == n1) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %~

static void _tilde(void)
{
    int64_t n1;

    n1 = fs_pop();

    fs_push(~n1);
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

    (n1 != 0) //
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

    (n1 != 0) //
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

    c1 = *f_str++;
    c_emit(c1);
    f_str++; // scan past terminating tick
}

// -----------------------------------------------------------------------
// format = %i

// for ansi terminals the first two parameters are ONE based not zero
// based so x/y oordinates start at 1 not 0

static void _i(void)
{
    params[0]++; // increment first two parameters
    params[1]++; // for ansi terminals
}

// -----------------------------------------------------------------------
// format = %s

static void _s(void)
{
    char *s;
    char c1;

    s = (char *)fs_pop();

    if(s != NULL)
    {
        while((c1 = *s++))
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

    if(s != NULL)
    {
        len = utf8_strlen(s);
        fs_push(len);
    }
}

// -----------------------------------------------------------------------
// return address of named variable

static int64_t *get_var_addr(void)
{
    int64_t *p;
    char c1;

    c1 = *f_str++;

    // this assumes that if it is not within the range 'a' to 'z'
    // then it is within the range 'A' to 'Z'

    p = ((c1 >= 'a') && (c1 <= 'z')) //
            ? &atoz[c1 - 'a']
            : &AtoZ[c1 - 'A'];

    return p;
}

// -----------------------------------------------------------------------
// format = %P

static void _P(void)
{
    int64_t *s;

    s = get_var_addr();

    *s = fs_pop();
}

// -----------------------------------------------------------------------
// format = &g

static void _g(void)
{
    int64_t *s;

    s = get_var_addr();

    fs_push(*s);
}

// -----------------------------------------------------------------------
// format = %{

static void _brace(void)
{
    int64_t n1;
    char c1;

    n1 = 0;

    while((c1 = *f_str++) != '}')
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
    while('%' != *f_str++)
        ;
}

// -----------------------------------------------------------------------
// format = %t

static void _t(void) // too much if/and/but loop nesting
{
    int64_t f1;
    char c1;
    int8_t nest; // not sure if any terminfo has nested %?

    nest = 0;

    f1 = fs_pop(); // if this is non 0 we dont do anything

    if(f1 == 0) // if it is 0 we skip past then part
    {
        for(;;)
        {
            // scan format string for next % char
            to_cmd();
            c1 = *f_str++;

            // if we are nesting if's count depth
            if(c1 == '?')
            {
                nest++;
            }

            // if we are at the else or endif....
            if((c1 == 'e') || (c1 == ';'))
            {
                if(0 == nest) // break out of loop if at else or endif
                {             // and we have scanned past all nested %?
                    break;
                }
                // we are within a nested %? -
                // must scan t0 %; before we break
                else if(c1 == ';')
                {
                    nest--;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------
// format = %e

static void _e(void)
{
    char c1;
    int8_t nest = 0;

    for(;;)
    {
        to_cmd();

        c1 = *f_str++;

        if(c1 == '?')
        {
            nest++;
        }

        if(c1 == ';')
        {
            if(nest == 0)
            {
                break;
            }
            nest--;
        }
    }
}

// -----------------------------------------------------------------------
// format = %d

static void _d(void)
{
    int64_t n1, n2;
    n1 = fs_pop();

    // this is a bug, if we are mid escape sequence we cant flush
    // a partial now then the rest later
    if((0xffff - num_esc) < 4)
    {
        flush();
    }

    switch(digits)
    {
        case 2:
            n2 = snprintf(&esc_buff[num_esc], 4, "%02" PRIu64, n1);
            break;
        case 3:
            n2 = snprintf(&esc_buff[num_esc], 4, "%03" PRIu64, n1);
            break;
        default:
            n2 = snprintf(&esc_buff[num_esc], 4, "%" PRIu64, n1);
    }

    num_esc += n2;
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

    c1 = *f_str++; // get parameter number from format string
    c1 &= 0x0f;    // 1' to '9'
    c1--;          // 0 to 8

    fs_push(params[c1]);
}

// -----------------------------------------------------------------------

static INLINE char next_c(void)
{
    char c1;

    digits = 1;
    c1 = *f_str++;

    if((c1 == '2') || (c1 == '3'))
    {
        digits = (c1 & 0x0f);
        // this should be a d ... should i test that?
        c1 = *f_str++;
    }
    return c1;
}

// -----------------------------------------------------------------------
// terminfo format string % codes

static const switch_t p_codes[] = {
    { '%', &_percent }, { 'p', &_p },      { 'd', &_d },
    { 'c', &_c },       { 'i', &_i },      { 's', &_s },
    { 'l', &_l },       { 'A', &_andl },   { '&', &_and },
    { 'O', &_orl },     { '|', &_or },     { '!', &_bang },
    { '~', &_tilde },   { '^', &_caret },  { '+', &_plus },
    { '-', &_minus },   { '*', &_star },   { '/', &_slash },
    { 'm', &_mod },     { '=', &_equals }, { '>', &_greater },
    { '<', &_less },    { 0x27, &_tick },  { '{', &_brace },
    { 'P', &_P },       { 'g', &_g },      { 't', &_t },
    { 'e', &_e },       { '?', &noop },    { ';', &noop },
};

#define PCOUNT (sizeof(p_codes) / sizeof(p_codes[0]))

// -----------------------------------------------------------------------
// silly inlined wrapper function to allow use of ternary below (ftw)

static INLINE void wrapper(const switch_t *s, size_t size, int32_t c)
{
    re_switch(s, size, c);
}

// -----------------------------------------------------------------------
// parse format string pointed to by f_str

// this function is called directly when ever we implement a hard coded
// format string such as for rgb colors. otherwise it would be static

void parse_format(void)
{
    char c1;

    while((c1 = *f_str++))
    {
        (c1 == '%') //
            ? wrapper(p_codes, PCOUNT, next_c())
            : c_emit(c1);
    }
}

// -----------------------------------------------------------------------
// parse a terminfo format string from the terminfo files strings section

void format(int16_t i)
{
    i = ti_strings[i];

    // it is not an error for a format string to be blank
    if(i != -1)
    {
        f_str = &ti_table[i];
        parse_format();
    }
}

// =======================================================================
