// test_ctrl_c.c  - tests for uC_ctrl_c_off() / uC_ctrl_c_on()
// -----------------------------------------------------------------------
// uC_init_terminal() works on STDIN_FILENO, so the test gives itself a pty
// to own rather than depending on how the suite was invoked.  the master
// end doubles as a keyboard: a byte written there is a keypress.

#include <pty.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#include "unity/unity.h"
#include "uCurses.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

static int master;
static struct termios original;

// -----------------------------------------------------------------------

void setUp(void)
{
    int slave;

    openpty(&master, &slave, NULL, NULL, NULL);
    dup2(slave, STDIN_FILENO);
    close(slave);

    tcgetattr(STDIN_FILENO, &original);
}

// -----------------------------------------------------------------------

void tearDown(void)
{
    uC_ctrl_c_on();
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
    close(master);
}

// -----------------------------------------------------------------------

static struct termios current(void)
{
    struct termios t;

    tcgetattr(STDIN_FILENO, &t);

    return t;
}

// -----------------------------------------------------------------------
// the default is unchanged: Ctrl-C is still the terminal's INTR character
// and still raises SIGINT

void test_default_leaves_intr_alone(void)
{
    uC_init_terminal();

    TEST_ASSERT_EQUAL_UINT8(original.c_cc[VINTR], current().c_cc[VINTR]);
    TEST_ASSERT_TRUE((current().c_lflag & ISIG) != 0);
}

// -----------------------------------------------------------------------

void test_off_disables_intr(void)
{
    uC_init_terminal();
    uC_ctrl_c_off();

    TEST_ASSERT_EQUAL_UINT8(_POSIX_VDISABLE, current().c_cc[VINTR]);
}

// -----------------------------------------------------------------------
// the whole point of disabling VINTR rather than clearing ISIG: Ctrl-\ and
// Ctrl-Z survive, so a wedged application can still be escaped

void test_off_keeps_quit_and_susp(void)
{
    uC_init_terminal();
    uC_ctrl_c_off();

    TEST_ASSERT_TRUE((current().c_lflag & ISIG) != 0);
    TEST_ASSERT_EQUAL_UINT8(original.c_cc[VQUIT], current().c_cc[VQUIT]);
    TEST_ASSERT_EQUAL_UINT8(original.c_cc[VSUSP], current().c_cc[VSUSP]);
}

// -----------------------------------------------------------------------

void test_on_restores_intr(void)
{
    uC_init_terminal();
    uC_ctrl_c_off();
    uC_ctrl_c_on();

    TEST_ASSERT_EQUAL_UINT8(original.c_cc[VINTR], current().c_cc[VINTR]);
}

// -----------------------------------------------------------------------
// NOT TESTED HERE: uC_restore_terminal() putting the character back on the
// way out.  it calls uC_rmkx(), so it needs a live terminfo and segfaults
// without one, and this suite has no terminfo fixture.  what makes it
// correct is that term_save keeps the original character - which is what
// the two tests below measure from the outside.

// -----------------------------------------------------------------------
// calling before uC_init_terminal() records the choice and applies it when
// the terminal is configured

void test_off_before_init_is_honoured(void)
{
    uC_ctrl_c_off();
    uC_init_terminal();

    TEST_ASSERT_EQUAL_UINT8(_POSIX_VDISABLE, current().c_cc[VINTR]);
}

// -----------------------------------------------------------------------
// re-initialising without restoring first - the shell-out path - must not
// learn the disabled character as the one to put back

void test_reinit_does_not_lose_the_intr_char(void)
{
    uC_ctrl_c_off();
    uC_init_terminal();
    uC_init_terminal();
    uC_ctrl_c_on();

    TEST_ASSERT_EQUAL_UINT8(original.c_cc[VINTR], current().c_cc[VINTR]);
}

// -----------------------------------------------------------------------
// end to end: with the INTR character disabled, Ctrl-C is a readable byte

void test_ctrl_c_arrives_as_a_byte(void)
{
    struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
    uint8_t k = 0;

    uC_init_terminal();
    uC_ctrl_c_off();

    TEST_ASSERT_EQUAL_INT(1, write(master, "\003", 1));
    TEST_ASSERT_EQUAL_INT(1, poll(&pfd, 1, 1000));
    TEST_ASSERT_EQUAL_INT(1, read(STDIN_FILENO, &k, 1));
    TEST_ASSERT_EQUAL_UINT8(0x03, k);
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_default_leaves_intr_alone);
    RUN_TEST(test_off_disables_intr);
    RUN_TEST(test_off_keeps_quit_and_susp);
    RUN_TEST(test_on_restores_intr);
    RUN_TEST(test_off_before_init_is_honoured);
    RUN_TEST(test_reinit_does_not_lose_the_intr_char);
    RUN_TEST(test_ctrl_c_arrives_as_a_byte);
    return UNITY_END();
}

// =======================================================================
