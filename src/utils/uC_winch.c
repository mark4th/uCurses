// uC_winch.h   - uCurses sigwinch handling
// -----------------------------------------------------------------------

#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include "uCurses.h"

// -----------------------------------------------------------------------

user_winch_t user_winch;
API bool winch;
struct sigaction old_action;

// -----------------------------------------------------------------------

void winch_handler(int sig)
{
    (void) sig;

    if (winch == false)
    {
        winch = true;           // set global flag == winch signal received

        if (user_winch != NULL)
        {
            (user_winch)();     // dont do "too much" in here
        }
    }
}

// -----------------------------------------------------------------------


void init_winch(void)
{
    struct sigaction new_action;

    winch = false;

    new_action.sa_handler = winch_handler;

    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGWINCH, NULL, &old_action);

    if (old_action.sa_handler != SIG_IGN)
    {
        sigaction(SIGWINCH, &new_action, NULL);
    }
}

// -----------------------------------------------------------------------

void de_init_winch(void)
{
    if (old_action.sa_handler != SIG_IGN)
    {
        sigaction(SIGWINCH, &old_action, NULL);
    }

}

// -----------------------------------------------------------------------

API void uC_register_winch(user_winch_t handler)
{
    if (handler != NULL)
    {
        user_winch = handler;
    }
}

// -----------------------------------------------------------------------

API void uC_deregister_winch(user_winch_t handler)
{
    if (user_winch == handler)
    {
        user_winch = NULL;
    }
}

// =======================================================================
