// uC_util.c
// -----------------------------------------------------------------------

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "uCurses.h"
#ifdef UC_MOUSE
#include "uC_mouse.h"
#endif
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
// FNV-1 on utf8 strings

API int32_t uC_fnv_hash(uint8_t *s)
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

API void uC_init_terminal(void)
{
    tcgetattr(STDIN_FILENO, &term_save);
    term = term_save;
    term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// -----------------------------------------------------------------------

API void uC_restore_terminal(void)
{
#ifdef UC_MOUSE
    uC_mouse_disable();
#endif
    uC_rmkx();
    tcsetattr(STDIN_FILENO, TCSANOW, &term_save);
    uC_curon();
    uC_terminfo_flush();
}

// -----------------------------------------------------------------------

static void (*fatal_handler)(const char *msg) = NULL;

// -----------------------------------------------------------------------
// install a handler called just before the library exits on a fatal error.
// the handler may longjmp() out or call exit() itself to suppress the
// library's own exit(1).  if it returns, exit(1) is still called.

API void uC_set_fatal_handler(void (*fp)(const char *msg))
{
    fatal_handler = fp;
}

// -----------------------------------------------------------------------

API __attribute__((noreturn)) void uC_abort(const char *msg)
{
    uCurses_deInit();
    if (fatal_handler)
    {
        fatal_handler(msg);
    }
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

// -----------------------------------------------------------------------
// HPA to rightmost column, VPA to bottom row, then DSR (query position).
// Terminal clamps the cursor to its actual boundary so the CPR response
// ESC[rows;colsR gives the true terminal dimensions.  More reliable than
// ioctl(TIOCGWINSZ) which can lag after a resize.

API void uC_get_console_size(uint16_t *width, uint16_t *height)
{
    char buf[32];
    int  i = 0;
    int  r = 24, c = 80;

    ssize_t n;

    uC_terminfo_flush();

    n = write(STDOUT_FILENO, "\033[9999G\033[9999d\033[6n", 18);
    (void)n;

    while (i < (int)(sizeof(buf) - 1))
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) { break; }
        if (buf[i++] == 'R') { break; }
    }
    buf[i] = '\0';

    sscanf(buf, "\033[%d;%dR", &r, &c);

    *width  = (uint16_t)c;
    *height = (uint16_t)r;
}

// -----------------------------------------------------------------------
// dont use this directly, use uC_ASSERT(f, msg); macro

API void uC_assert(bool f, const char *file, int line, const char *msg)
{
    char buf[256];

    if (!f)
    {
        snprintf(buf, sizeof(buf), "%s:%d %s",
            file ? file : "?", line, msg ? msg : "");
        uC_abort(buf);
    }
}

// =======================================================================
