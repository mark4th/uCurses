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
    ti_vars->num_k = 0;

    do
    {
        if (ti_vars->num_k == KEY_BUFF_SZ)
        {
            break;
        }
        ti_vars->keybuff[ti_vars->num_k++] = read_key();
    } while (uC_test_keys() != 0);
}

// =======================================================================
