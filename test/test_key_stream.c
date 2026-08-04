// test_key_stream.c  - integration test for the STREAMING read path
// -----------------------------------------------------------------------
// Exercises the real fd byte source (uC_key_fd_source) driving sm_run():
// poll() for the next byte, read() it, append to keybuff, and the ~25ms
// end-of-sequence timeout that terminates a sequence.  A pipe stands in for
// the tty — its read end is duped onto fd 0 (polled) and fd 1 (read by
// read_key()), matching how the library polls stdin and reads stdout's tty.
//
// uC_key_read.c + uC_key_sm.c are compiled straight in (both hold hidden
// symbols); we supply our own ti_vars.

#include <string.h>
#include <unistd.h>

#include "unity/unity.h"
#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_keys.h"

// -----------------------------------------------------------------------

static ti_vars_t tv;
ti_vars_t *ti_vars = &tv;

extern uint8_t key_mods;

static int pipe_wr = -1;        // write end — feed bytes "from the terminal"

// Unity reports through this (fds 0/1 are the pipe): write to fd 2 instead
void unity_putc(int c)
{
    char b = (char)c;
    if (write(2, &b, 1) < 0) { /* nothing we can do */ }
}

void setUp(void)
{
    memset(&tv, 0, sizeof(tv));
    key_mods = 0;
}

void tearDown(void) {}

// -----------------------------------------------------------------------
// feed bytes into the pipe, then run the SM as uC_key_raw() would after it
// has read a leading ESC: keybuff[0]=0x1b, num_k=1, pull the rest live.

static int16_t feed_after_esc(const void *bytes, size_t n)
{
    if (n != 0)
    {
        ssize_t w = write(pipe_wr, bytes, n);
        TEST_ASSERT_EQUAL_INT((ssize_t)n, w);
    }
    tv.keybuff[0] = 0x1b;
    tv.num_k      = 1;
    return sm_run(uC_key_fd_source, NULL);
}

// -----------------------------------------------------------------------

void test_stream_arrow_up(void)
{
    TEST_ASSERT_EQUAL_INT16(K_CUU1, feed_after_esc("[A", 2));
    TEST_ASSERT_EQUAL_UINT8(0, key_mods);
    // the source must have appended the sequence bytes to keybuff
    TEST_ASSERT_EQUAL_INT16(3, tv.num_k);
    TEST_ASSERT_EQUAL_UINT8('[', tv.keybuff[1]);
    TEST_ASSERT_EQUAL_UINT8('A', tv.keybuff[2]);
}

void test_stream_ctrl_right(void)
{
    TEST_ASSERT_EQUAL_INT16(K_CUF1, feed_after_esc("[1;5C", 5));
    TEST_ASSERT_EQUAL_UINT8(KMOD_CTRL, key_mods);
}

void test_stream_delete_tilde(void)
{
    TEST_ASSERT_EQUAL_INT16(K_DCH1, feed_after_esc("[3~", 3));
}

void test_stream_f1_ss3(void)
{
    TEST_ASSERT_EQUAL_INT16(K_F1, feed_after_esc("OP", 2));
}

// the timeout branch: nothing follows the ESC -> bare ESC after ~25ms
void test_stream_bare_esc_times_out(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_DIRECT, feed_after_esc(NULL, 0));
    TEST_ASSERT_EQUAL_INT16(1, tv.num_k);
    TEST_ASSERT_EQUAL_UINT8(0x1b, tv.keybuff[0]);
}

// Alt+b: one byte follows, then the window closes -> Alt+char
void test_stream_alt_b(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_DIRECT, feed_after_esc("b", 1));
    TEST_ASSERT_EQUAL_UINT8(KMOD_ALT, key_mods);
    TEST_ASSERT_EQUAL_UINT8('b', tv.keybuff[0]);
    TEST_ASSERT_EQUAL_INT16(1, tv.num_k);
}

// mouse introducer: not a key, but the whole report is drained into keybuff
// so uC_mouse_parse() (which reads from index 3) still sees it.
void test_stream_x10_mouse_drained(void)
{
    TEST_ASSERT_EQUAL_INT16(SM_UNHANDLED, feed_after_esc("[M @!", 5));
    TEST_ASSERT_EQUAL_UINT8('[', tv.keybuff[1]);
    TEST_ASSERT_EQUAL_UINT8('M', tv.keybuff[2]);
    TEST_ASSERT_EQUAL_INT16(6, tv.num_k);   // ESC [ M + 3 report bytes
}

// -----------------------------------------------------------------------

int main(void)
{
    int fds[2];

    if (pipe(fds) != 0)
    {
        return 1;
    }
    // read end feeds both the poll fd (0) and the read fd (1)
    dup2(fds[0], 0);
    dup2(fds[0], 1);
    pipe_wr = fds[1];

    UNITY_BEGIN();
    RUN_TEST(test_stream_arrow_up);
    RUN_TEST(test_stream_ctrl_right);
    RUN_TEST(test_stream_delete_tilde);
    RUN_TEST(test_stream_f1_ss3);
    RUN_TEST(test_stream_bare_esc_times_out);
    RUN_TEST(test_stream_alt_b);
    RUN_TEST(test_stream_x10_mouse_drained);
    return UNITY_END();
}

// =======================================================================
