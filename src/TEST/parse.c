#include <libchordtest/test.h>
#include "src/parse.c"

// ------------------------------------------------------------------------
// required initialization for tests

static void src_parse_init(void)
{
    uCurses_init();
}

// ------------------------------------------------------------------------

#include <libchord/newframe.h>
#define frame check_c_emit

void our(check_c_emit)(char sent)
{
    uint16_t prev_num_esc = num_esc;
    c_emit(sent);
    char got = esc_buff[prev_num_esc];

    log_debug(("sent = %02x, got = %02x, num_esc = %d, prev_num_esc = %d",
        sent, got, num_esc, prev_num_esc));

    ut_assert(num_esc == prev_num_esc + 1);
    ut_assert(got == sent);
}

TEST("c_emit", "check for add to esc_buff")
{
    src_parse_init();
    our(check_c_emit)(' ');
    our(check_c_emit)('*');
}

// -----------------------------------------------------------------------

void our(check_fs_push)(int64_t sent)
{
    uint16_t prev_fsp = fsp;
    fs_push(sent);
    int64_t got = fstack[prev_fsp];

    log_debug(("sent = %" PRIx64 ", got = %" PRIx64 ", fsp = %d prev_fsp = %d",
        sent, got, fsp, prev_fsp));

    ut_assert(fsp == prev_fsp + 1);
    ut_assert(got == sent);
}

TEST("fs_push", "check format string statck push operation")
{
    src_parse_init();
    our(check_fs_push)(0x12);
    our(check_fs_push)(0x34);
}

// -----------------------------------------------------------------------

void our(check_fs_pop)(int64_t sent)
{
    fs_push(sent);

    uint16_t prev_fsp = fsp;
    int64_t got = fs_pop();

    log_debug(("sent = %" PRIx64 ", got = %" PRIx64 ", fsp = %d prev_fsp = %d",
        sent, got, fsp, prev_fsp));

    ut_assert(fsp == prev_fsp - 1);
    ut_assert(got == sent);
}

TEST("fs_push", "check format string statck push operation")
{
    src_parse_init();
    our(check_fs_push)(0x12);
    our(check_fs_push)(0x34);
}

// -----------------------------------------------------------------------

void our(check_percent)(void)
{
    uint16_t prev_num_esc = num_esc;
    _percent();
    char sent = '%';
    char got = esc_buff[prev_num_esc];

    log_debug(("sent = %02x, got = %02x, num_esc = %d, prev_num_esc = %d",
        sent, got, num_esc, prev_num_esc));

    ut_assert(num_esc == prev_num_esc + 1);
    ut_assert(got == sent);
}

TEST("check_percent", "verify addition of % char to escape buffer")
{
    src_parse_init();
    our(check_percent)();
}

// -----------------------------------------------------------------------

void our(check_and(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);
    _and();
    rv = fs_pop();

    log_debug(("sent = %" PRIx64 ":%" PRIx64 " expected = %" PRIx64 " got = %" PRIx64,
        n1, n2, rv, n1 & n2));

    ut_assert(rv == (n1 & n2));
}

TEST("_and", "check format string 'and' function")
{
    src_parse_init();
    our(check_and(0x55, 0xff));
    our(check_and(3, 2));
}

// -----------------------------------------------------------------------

void our(check_and_logical(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);
    _andl();
    rv = fs_pop();

    log_debug(("sent = %" PRIx64 ":%" PRIx64 " expected = %" PRIx64 " got = %d",
        n1, n2, rv, n1 && n2));

    ut_assert(rv == (n1 && n2));
}

TEST("_andl", "check format string logical 'and' function")
{
    src_parse_init();
    our(check_and_logical(0x55, 0xff));
    our(check_and_logical(1, 0));
}

// -----------------------------------------------------------------------

void our(check_or(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);
    _or();
    rv = fs_pop();

    log_debug(("sent = %" PRIx64 ":%" PRIx64 " expected = %" PRIx64 " got = %" PRIx64,
        n1, n2, rv, (n1 | n2)));

    ut_assert(rv == (n1 | n2));
}

TEST("_or", "check format string 'or' function")
{
    src_parse_init();
    our(check_or(0x55, 0xff));
    our(check_or(1, 0));
}

// -----------------------------------------------------------------------

void our(check_or_logical(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);
    _orl();
    rv = fs_pop();

    log_debug((
        "sent = %"  PRIx64 ":%" PRIx64
        " expected = %" PRIx64 " got = %d",
        n1, n2, rv, (n1 || n2)));

    ut_assert(rv == (n1 || n2));
}

TEST("_andl", "check format string logical 'or' function")
{
    src_parse_init();
    our(check_or_logical(0x55, 0xff));
    our(check_or_logical(1, 0));
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

// =======================================================================

