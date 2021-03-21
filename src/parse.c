// parse.c  - uCurses terminfo format string parsing
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "h/tui.h"
#include "h/uCurses.h"

// -----------------------------------------------------------------------
// c switch statements are FUGLY

typedef void (*opt_t)(void);

typedef struct
{
    uint8_t option;
    opt_t vector;
} switch_t;

// -----------------------------------------------------------------------
// for format string % commands that dont actually need to do anything

static void noop(void){;}

// -----------------------------------------------------------------------

char *esc_buff;             // format string compilation output buffer
uint32_t num_esc;           // max of 64k of compiled escape seq bytes
uint64_t params[MAX_PARAM]; // format string parametesr

static uint8_t fsp;         // stack pointer for ...
static uint64_t fstack[5];  // format string stack

const char *f_str;          // pointer to next character of format string
static uint8_t digits;      // number of digits for %d (2 or 3)

static uint64_t atoz[26];   // named format string variables
static uint64_t AtoZ[26];

// -----------------------------------------------------------------------
// addresses within memory mapped terminfo file

extern char *ti_table;   // array of offsets within following
extern uint16_t *ti_strings;

// -----------------------------------------------------------------------
// debug

// void dump_buff(void)
// {
//     int i = 0;

//     for(i = 0; i != num_esc; i++)
//     {
//         if((i & 7) == 0)
//         {
//             printf("\r\n");
//         }
//         printf(" %02x ", esc_buff[i]);
//     }
//     printf("\r\n");
// }

// -----------------------------------------------------------------------
// debug

// void print_format(uint8_t *p)
// {
//     while(*p)
//     {
//         (*p < 0x20)
//           ? printf(" %02x ", *p)
//           : printf("%c", *p);
//         p++;
//     }
//     printf("\r\n");
// }

// -----------------------------------------------------------------------

void flush(void)
{
    ssize_t n;

    n =  write(1, &esc_buff[0], num_esc);
    num_esc = 0;

    if(n < 0)
    {
        // log warning? try again?
    }
}

// -----------------------------------------------------------------------
// write one character of escape sequence out to compilation buffer

void c_emit(uint8_t c1)
{
    esc_buff[num_esc++] = c1;

    if(num_esc == 0xffff)
    {
        flush();
    }
}

// -----------------------------------------------------------------------
// push item onto format string stack (terminfo is RPN!!)

static void fs_push(uint64_t n)
{
    if(fsp != 5)
    {
        fstack[fsp++] = n;
        return;
    }
    // methinks this might could be an internal error
    // abort "uCurses format string stack overflow"
}

// -----------------------------------------------------------------------
// pop item off of format string stack

static uint64_t fs_pop(void)
{
    return (0 != fsp)
        ? fstack[--fsp] :
        0;                  // also an internal error?
}

// -----------------------------------------------------------------------
// terminfo format string operators
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// format = %%

static void _percent(void)
{
    c_emit('%');
}

// -----------------------------------------------------------------------
// format = %A or %&

static void _and(void)
{
    uint64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 & n2);
}

// -----------------------------------------------------------------------
// format = %O or %|

static void _or(void)
{
    uint64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 | n2);
}

// -----------------------------------------------------------------------
// format = %!

static void _bang(void)
{
    uint64_t n1;

    n1 = fs_pop();
    n1 = (0 == n1) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %~

static void _tilde(void)
{
    uint64_t n1;

    n1 = fs_pop();

    fs_push(~n1);
}

// -----------------------------------------------------------------------
// format = %^

static void _caret(void)
{
    uint64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 ^ n2);
}

// -----------------------------------------------------------------------
// format = %+

static void _plus(void)
{
    uint64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n1 + n2);
}

// -----------------------------------------------------------------------
// format = %-

static void _minus(void)
{
    uint64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n2 - n1);
}

// -----------------------------------------------------------------------
// format = %*

static void _star(void)
{
    uint64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    fs_push(n2 * n1);
}

// -----------------------------------------------------------------------
// format = %/

static void _slash(void)
{
    uint64_t n1, n2;

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
    uint64_t n1, n2;

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
    uint64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    n1 = (n2 == n1) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %>

static void _greater(void)
{
    uint64_t n1, n2;

    n1 = fs_pop();
    n2 = fs_pop();

    n1 = (n2 > n1) ? 1 : 0;

    fs_push(n1);
}

// -----------------------------------------------------------------------
// format = %<

static void _less(void)
{
    uint64_t n1, n2;

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
    f_str++;
}

// -----------------------------------------------------------------------
// format = %i

static void _i(void)
{
    params[0]++;            // increment first two parameters
    params[1]++;            // for ansi terminals
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
    uint16_t len;

    s = (char *)fs_pop();
    len = utf8_strlen(s);

    if(s != NULL)
    {
        fs_push(len);
    }
}

// -----------------------------------------------------------------------
// return address of named variable

static uint64_t *get_var_addr(void)
{
    uint64_t *p;
    char c1;

    c1 = *f_str++;

    // this assumes that if it is not within the range 'a' to 'z'
    // then it is within the range 'A' to 'Z'

    p = ((c1 >= 'a') && (c1 <= 'z'))
      ? &atoz[c1 - 'a']
      : &AtoZ[c1 - 'A'];

    return p;
}

// -----------------------------------------------------------------------
// format = %P

static void _P(void)
{
    uint64_t *s;

    s = get_var_addr();

    *s = fs_pop();
}

// -----------------------------------------------------------------------
// format = &g

static void _g(void)
{
    uint64_t *s;

    s = get_var_addr();

    fs_push(*s);
}

// -----------------------------------------------------------------------
// format = %{

static void _brace(void)
{
    uint64_t n1;
    char c1;

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
// skip forward in format string to next % command

static void to_cmd(void)
{
    while('%' != *f_str++)
      ;
}

// -----------------------------------------------------------------------
// format = %t

static void _t(void)        // too much if/and/but loop nesting
{
    uint64_t f1;
    char c1;
    uint8_t nest;           // not sure if any terminfo has nested %?

    nest = 0;

    f1 = fs_pop();          // if this is non 0 we dont do anything

    if(f1 == 0)             // if it is 0 we skip past then part
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
                if(0 == nest)  // break out of loop if at else or endif
                {              // and we have scanned past all nested %?
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
    uint8_t nest=0;

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
    uint64_t n1, n2;
    n1 = fs_pop();

    uint16_t available = 0xffff - num_esc;

    if(available <= n1)
    {
        flush();
        available = 0xffff;
    }

    switch(digits)
    {
        case 2:
            n2 = snprintf(&esc_buff[num_esc], available, "%02" PRIu64, n1);
            break;
        case 3:
            n2 = snprintf(&esc_buff[num_esc], available, "%03" PRIu64, n1);
            break;
        default :
            n2 = snprintf(&esc_buff[num_esc], available, "%" PRIu64, n1);
    }

    num_esc += n2;
}

// -----------------------------------------------------------------------
// format = %c

static void _c(void)
{
    uint64_t c1;

    c1 = fs_pop();
    c_emit(c1);
}

// -----------------------------------------------------------------------
// format = %p

static void _p(void)
{
    uint8_t c1;

    c1 = *f_str++;          // get parameter number from format string
    c1 &= 0x0f;             // '1' to '9'
    c1--;                   // 0 to 8
    c1 = params[c1];
    fs_push(c1);
}

// -----------------------------------------------------------------------

static uint8_t next_c(void)
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

const switch_t p_codes[] =
{
    { '%', &_percent }, { 'p', &_p },      { 'd', &_d },
    { 'c', &_c },       { 'i', &_i },      { 's', &_s },
    { 'l', &_l },       { 'A', &_and },    { '&', &_and },
    { 'O', &_or },      { '|', &_or },     { '!', &_bang },
    { '~', &_tilde },   { '^', &_caret },  { '+', &_plus },
    { '-', &_minus },   { '*', &_star },   { '/', &_slash },
    { 'm', &_mod },     { '=', &_equals }, { '>', &_greater },
    { '<', &_less },    { 0x27, &_tick },  { '{', &_brace },
    { 'P', &_P },       { 'g', &_g },      { 't', &_t },
    { 'e', &_e },       { '?', &noop },    { ';', &noop },
};

#define PCOUNT sizeof(p_codes) / sizeof(p_codes[0])

// -----------------------------------------------------------------------

static void command(void)
{
    char c1;
    int n = PCOUNT;

    const switch_t *s = &p_codes[0];

    c1 = next_c();

    while((0 != n) && c1 != s->option)
    {
       s++; n--;
    }
    // the only way this could fail here is if the terminfo fornat string
    // has been corrupted or if terminfo format strings have been extended
    // some time in the future.
    if(c1 == s->option)
    {
        (s->vector)();
    }
    else
    {
        // snafu!
    }
}

// -----------------------------------------------------------------------
// parse format string pointed to by f_str

// this function is called directly when ever we implement a hard coded
// format string such as for gray scales and rgb colors. otherwise it
// would be static

void do_parse_format(void)
{
    char c1;

    while((c1 = *f_str++))
    {
        (c1 == '%')
            ? command()
            : c_emit(c1);
    }
}

// -----------------------------------------------------------------------

void format(uint16_t i)
{
    i = ti_strings[i];

    // it is not an error for a format string to be blank
    if(i != 0xffff)
    {
        f_str = &ti_table[i];
        do_parse_format();
    }
}

// =======================================================================
