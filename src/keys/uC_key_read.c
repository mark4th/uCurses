// uC_key_read.c
// -----------------------------------------------------------------------

#include <poll.h>
#include <inttypes.h>
#include <unistd.h>

#include "uCurses.h"
#include "uC_keys.h"
#include "uC_terminfo.h"

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;

// -----------------------------------------------------------------------

static struct pollfd pfd =
{
    0,                      // stdin
    POLLIN,                 // want to know when data is available
    0
};

#define ESC_SEQUENCE_POLL_MS (25)

// -----------------------------------------------------------------------
// returns 0 = no keys available, greater than zero = keys available

API int8_t uC_test_keys(void)
{
    int8_t k;

    if (ti_vars->stuffed == true)
    {
        ti_vars->stuffed = false;
        return ti_vars->num_k;
    }

    k = poll(&pfd, 1, 0);

    if (k < 0)
    {
        k = 0;              // no keys pressed
    }

    return k;
}

// -----------------------------------------------------------------------
// read single keypress

static int8_t read_key(void)
{
    int n;
    uint8_t k;

    do
    {
        n = read(1, &k, 1);
        // todo this might be bad :)
    } while (n == -1);

    return k;
}

// -----------------------------------------------------------------------
// read escape sequence or singke keypress character

void uC_read_keys(void)
{
    bool read_more = true;

    ti_vars->num_k = 0;

    do
    {
        int poll_rc;

        if (ti_vars->num_k == KEY_BUFF_SZ)
        {
            break;
        }
        ti_vars->keybuff[ti_vars->num_k++] = read_key();

        poll_rc = poll(&pfd, 1,
            (ti_vars->keybuff[0] == 0x1b) ? ESC_SEQUENCE_POLL_MS : 0);
        read_more = poll_rc != 0;
    } while (read_more);
}

// -----------------------------------------------------------------------
// read the first byte of a keypress (blocking) into keybuff[0].  a non-ESC
// byte is already a complete keypress; an ESC (0x1b) starts a sequence whose
// remaining bytes the state machine pulls one at a time via the source below.
// this is the streaming replacement for uC_read_keys()' greedy whole-sequence
// buffering: it reads exactly one byte and lets the SM decide what follows.

uint8_t uC_read_key(void)
{
    ti_vars->keybuff[0] = read_key();
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
    (void)ctx;

    if (poll(&pfd, 1, timeout_ms) <= 0)
    {
        return -1;                      // no byte within the window
    }
    if (ti_vars->num_k >= KEY_BUFF_SZ)
    {
        return -1;                      // buffer full — stop pulling
    }

    ti_vars->keybuff[ti_vars->num_k++] = read_key();

    return ti_vars->keybuff[ti_vars->num_k - 1];
}

// =======================================================================
