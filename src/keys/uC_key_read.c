// uC_key_read.c
// -----------------------------------------------------------------------

#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <stdbool.h>
#include <unistd.h>

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_terminfo.h"

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;

// -----------------------------------------------------------------------

static bool input_closed;

// -----------------------------------------------------------------------

static struct pollfd pfd =
{
    STDIN_FILENO,
    POLLIN,                 // want to know when data is available
    0
};

// -----------------------------------------------------------------------
// returns 0 = no keys available, greater than zero = keys available

API int8_t uC_test_keys(void)
{
    int k;

    if (ti_vars->stuffed == true)
    {
        ti_vars->stuffed = false;
        return ti_vars->num_k;
    }

    if (input_closed)
    {
        return 0;
    }

    pfd.revents = 0;
    k = poll(&pfd, 1, 0);

    if ((k <= 0) || ((pfd.revents & POLLIN) == 0))
    {
        return 0;           // no input; HUP/ERR is not a keypress
    }

    return 1;
}

// -----------------------------------------------------------------------
// read single keypress

static int read_key(void)
{
    ssize_t n;
    uint8_t k;

    do
    {
        n = read(STDIN_FILENO, &k, 1);
    } while ((n < 0) && (errno == EINTR));

    if (n == 0)
    {
        input_closed = true;
        return -1;          // end of file
    }

    if (n < 0)
    {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
        {
            input_closed = true;
        }
        return -1;          // no byte or a permanent input error
    }

    return k;
}

// -----------------------------------------------------------------------
// read the first byte of a keypress (blocking) into keybuff[0].  a non-ESC
// byte is already a complete keypress; an ESC (0x1b) starts a sequence whose
// remaining bytes the state machine pulls one at a time via the source below.
// it reads exactly one byte and lets the SM decide what follows.

uint8_t uC_read_key(void)
{
    int key;

    key = read_key();

    if (key < 0)
    {
        ti_vars->keybuff[0] = UC_KEY_NONE;
        ti_vars->num_k = 0;
        return UC_KEY_NONE;
    }

    ti_vars->keybuff[0] = (uint8_t)key;
    ti_vars->num_k      = 1;

    return ti_vars->keybuff[0];
}

// -----------------------------------------------------------------------
// streaming byte source for the state machine (sm_run()).  returns the next
// stdin byte if one arrives within timeout_ms, or -1 when the window closes
// (a ~25ms gap => the escape sequence has ended: bare ESC, end of CSI, etc).
// each byte is appended to keybuff so the mouse parser (which reads keybuff/
// num_k from index 3) and the SM_DIRECT path still see the raw sequence.

int uC_key_fd_source(void *ctx, int timeout_ms)
{
    int key;

    (void)ctx;

    if (input_closed)
    {
        return -1;
    }

    pfd.revents = 0;

    if ((poll(&pfd, 1, timeout_ms) <= 0) ||
        ((pfd.revents & POLLIN) == 0))
    {
        return -1;                      // no byte within the window
    }
    if (ti_vars->num_k >= KEY_BUFF_SZ)
    {
        return -1;                      // buffer full — stop pulling
    }

    key = read_key();

    if (key < 0)
    {
        return -1;
    }

    ti_vars->keybuff[ti_vars->num_k++] = (uint8_t)key;

    return ti_vars->keybuff[ti_vars->num_k - 1];
}

// =======================================================================
