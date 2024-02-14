// util.c
// -----------------------------------------------------------------------

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_utf8.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

static struct termios term;
static struct termios term_save;

// -----------------------------------------------------------------------
// do nothing and do it well

API void uC_noop(void) { ; }

// -----------------------------------------------------------------------
// FNV-1a on utf8 strings

// actually i think this is just fnv-1  not 1a : fix later

API int32_t fnv_hash(char *s)
{
    uint32_t hash = FNV_BASIS;

    int16_t len;

    while (*s != '\0')
    {
        len = uC_utf8_char_length(s);

        while (len != 0)
        {
            hash += (hash << 1) + (hash << 4) + (hash << 7) +
                (hash << 8) + (hash << 24);
            hash ^= *s++;
            len--;
        }
    }

    return hash;
}

// -----------------------------------------------------------------------

API void uC_clock_sleep(int32_t when)
{
    struct timespec tv;
    struct timespec remain;
    int rv;

    do
    {
        tv.tv_sec  = 0;
        tv.tv_nsec = when;
        rv = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &remain);
        tv = remain;
    } while (EINTR == rv);
}

// -----------------------------------------------------------------------

API void uC_restore_terminal(void)
{
    // these flags should not have been cleared here, why do i suddenly
    // need to set them?
    term_save.c_lflag |= (ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term_save);
    uC_curon();
    uC_terminfo_flush();
}

// -----------------------------------------------------------------------

API void uC_init_terminal(void)
{
    tcgetattr(STDIN_FILENO, &term_save);
    term = term_save;
    term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// -----------------------------------------------------------------------

__attribute__((noreturn)) void xabort(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    uCurses_deInit();
    exit(1);
}

// -----------------------------------------------------------------------

void uC_assert(bool f, char *file, int line, char *msg)
{
    char *m = (msg != NULL) ? msg : "";

    if (f == false)
    {
        if (file != NULL)
        {
            fprintf(stderr, "%s:%d %s\n", file, line, m);
        }
        uCurses_deInit();
        exit(1);
    }
}

// =======================================================================
