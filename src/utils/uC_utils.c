// uC_util.c
// -----------------------------------------------------------------------

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_utf8.h"
#include "uC_utils.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------

static struct termios term;
static struct termios term_save;

// -----------------------------------------------------------------------
// do nothing and do it well

API void uC_noop(void) { ; }

// -----------------------------------------------------------------------

API void uC_ui_free(void *mem)
{
    uC_free(uC_MEM_ZONE_UI, mem);
}

// -----------------------------------------------------------------------
// FNV-1a on utf8 strings

// actually i think this is just fnv-1  not 1a : fix later

API int32_t fnv_hash(uint8_t *s)
{
    uint32_t hash = FNV_BASIS;

    int16_t len;

    if (s == NULL)
    {
        return 0;
    }

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

API void uC_clock_sleep(int32_t whence)
{
    int rv;
    struct timespec tv;

    tv.tv_sec  = 0;
    tv.tv_nsec = whence;

    do
    {
        rv = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &tv);
    } while (rv == EINTR);
}

// -----------------------------------------------------------------------

API void uC_restore_terminal(void)
{
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

API __attribute__((noreturn)) void uC_abort(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    uCurses_deInit();
    exit(1);
}

// -----------------------------------------------------------------------
// the story I hear now is that this is unreliable and that I should
// be setting the cursor position to something like x = 999, y = 999 (move
// in x first, then in y) then ask the terminal to report back to me
// what the actual cursor location is.
//
// I need to know if this ever actually fails because I am not implementing
// that garbage unless I legitimately need to.

API void uC_get_console_size(uint16_t *width, uint16_t *height)
{
    struct winsize w;

    ioctl(0, TIOCGWINSZ, &w);

    *width  = w.ws_col;
    *height = w.ws_row;
}

// -----------------------------------------------------------------------
// dont use this directly, use uC_ASSERT(f, msg); macro

API void uC_assert(bool f, char *file, int line, char *msg)
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
