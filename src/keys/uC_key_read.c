// key_read.c
// -----------------------------------------------------------------------

#include <poll.h>
#include <inttypes.h>
#include <unistd.h>

#include "uCurses.h"
#include "uC_keys.h"

// -----------------------------------------------------------------------

int8_t keybuff[KEY_BUFF_SZ];
int16_t num_k;
int8_t stuffed;

// -----------------------------------------------------------------------

static struct pollfd pfd =
{
    0,                      // stdin
    POLLIN,                 // want to know when data is available
    0
};

// -----------------------------------------------------------------------
// returns 0 = no keys available, 1 = keys available

API int8_t uC_test_keys(void)
{
    int k = stuffed;        // has a key been manually stuffed ?

    if (stuffed == 0)       // if not...
    {
        // see if any keys have been pressed
        k = poll(&pfd, 1, 0);
        if (k < 0)
        {
            k = 0;          // no keys pressed
        }
    }

    return (int8_t)k;
}

// -----------------------------------------------------------------------
// read single keypress

static int8_t read_key(void)
{
    int8_t k = stuffed;
    int n;

    if (stuffed == 0)
    {
        do
        {
            n = read(1, &k, 1);
            // todo this might be bad :)
        } while (n == -1);
    }

    stuffed = 0;
    return k;
}

// -----------------------------------------------------------------------
// read escape sequence or singke keypress character

void uC_read_keys(void)
{
    num_k = 0;

    do
    {
        if (num_k == KEY_BUFF_SZ) { break; }
        keybuff[num_k++] = read_key();
    } while (uC_test_keys() != 0);
}

// =======================================================================
