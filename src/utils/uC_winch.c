// uC_winch.c   - uCurses SIGWINCH handling
// -----------------------------------------------------------------------

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>

#include "uCurses.h"

// -----------------------------------------------------------------------

static user_winch_t user_winch;
static struct sigaction old_action;
static bool have_old_action;

API volatile sig_atomic_t winch;

// -----------------------------------------------------------------------

static void winch_handler(int sig)
{
    (void)sig;

    winch = 1;
}

// -----------------------------------------------------------------------

void init_winch(void)
{
    struct sigaction new_action;

    winch = 0;
    have_old_action = (sigaction(SIGWINCH, NULL, &old_action) == 0);

    if (have_old_action && (old_action.sa_handler == SIG_IGN))
    {
        return;
    }

    sigemptyset(&new_action.sa_mask);
    new_action.sa_handler = winch_handler;
    new_action.sa_flags = 0;

    sigaction(SIGWINCH, &new_action, NULL);
}

// -----------------------------------------------------------------------

void de_init_winch(void)
{
    if (have_old_action && (old_action.sa_handler != SIG_IGN))
    {
        sigaction(SIGWINCH, &old_action, NULL);
    }

    have_old_action = false;
    winch = 0;
}

// -----------------------------------------------------------------------

API bool uC_winch_pending(void)
{
    return winch != 0;
}

// -----------------------------------------------------------------------

API void uC_winch_ack(void)
{
    winch = 0;
}

// -----------------------------------------------------------------------

API bool uC_winch_dispatch(void)
{
    if (!uC_winch_pending())
    {
        return false;
    }

    uC_winch_ack();

    if (user_winch != NULL)
    {
        user_winch();
    }

    return true;
}

// -----------------------------------------------------------------------

API void uC_register_winch(user_winch_t handler)
{
    user_winch = handler;
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
