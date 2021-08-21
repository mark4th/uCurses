// parse.c -- unit tests
// -----------------------------------------------------------------------

#include <libchordtest/test.h>
#include "h/ut_assert.h"
#include "src/parse.c"

// ------------------------------------------------------------------------
// required initialization for tests

static void src_parse_init(void)
{
    uCurses_init();
}

static void src_parse_teardown(void)
{
    uCurses_deInit();
}

// ------------------------------------------------------------------------

#include <libchord/newframe.h>
#define frame test_c_emit

// ------------------------------------------------------------------------

void our(test_c_emit)(char sent)
{
    uint16_t prev_num_esc;
    char got;

    prev_num_esc = num_esc;

    c_emit(sent);

    got = esc_buff[prev_num_esc];

    ut_assert_op_int16x(num_esc, ==, prev_num_esc + 1);
    ut_assert_op_int16x(got, ==, sent);
}

TEST("c_emit", "Test add of character to terminfo esc_buff")
{
    src_parse_init();
    our(test_c_emit)('x');
    our(test_c_emit)('y');
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_fs_push)(int64_t sent)
{
    int8_t prev_fsp;
    int64_t got;

    prev_fsp = fsp;

    fs_push(sent);

    got = fstack[prev_fsp];

    ut_assert_op_int8(fsp, ==, prev_fsp + 1);
    ut_assert_op_int64x(got, ==, sent);
}

TEST("fs_push", "Test push to terminfo format string stack")
{
    src_parse_init();
    our(test_fs_push)(0x12);
    our(test_fs_push)(0x34);

    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_fs_pop)(int64_t sent)
{
    int8_t prev_fsp;
    int64_t got;

    fs_push(sent);

    prev_fsp = fsp;
    got = fs_pop();

    ut_assert_op_int8(fsp, ==, prev_fsp - 1);
    ut_assert_op_int64x(got, ==, sent);
}

TEST("fs_push", "Test pop from terminfo format string stack")
{
    src_parse_init();
    our(test_fs_push)(0x12);
    our(test_fs_push)(0x34);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_percent)(void)
{
    int16_t prev_num_esc;
    char sent;
    char got;

    prev_num_esc = num_esc;
    _percent();

    sent = '%';
    got = esc_buff[prev_num_esc];

    ut_assert_op_int16x(num_esc, ==, prev_num_esc + 1);
    ut_assert_op_int8(got, ==, sent);
}

TEST("_percent", "Test add of '%' to terminfo esc_buff")
{
    src_parse_init();
    our(test_percent)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_and)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _and();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 & n2));
}

TEST("_and", "Test terminfo format string 'and' operator")
{
    src_parse_init();
    our(test_and)(0x55, 0xff);
    our(test_and)(3, 2);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_and_logical)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _andl();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 && n2));
}

TEST("_andl", "Test teerminfo format string logical 'and' operator")
{
    src_parse_init();
    our(test_and_logical)(0x55, 0xff);
    our(test_and_logical)(1, 0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_or)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _or();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 | n2));
}

TEST("_or", "Test terminfo format string 'or' operator")
{
    src_parse_init();
    our(test_or)(0x55, 0xff);
    our(test_or)(1, 0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_or_logical)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _orl();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 || n2));
}

TEST("_orl", "Test terminfo format string logical 'or' operator")
{
    src_parse_init();
    our(test_or_logical)(0x55, 0xff);
    our(test_or_logical)(1, 0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_not)(int64_t n1)
{
    int64_t rv;

    fs_push(n1);

    _tilde();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, ~n1);
}

TEST("_tilde", "Test terminfo format string 'not' operator")
{
    src_parse_init();
    our(test_not)(0x55);
    our(test_not)(0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_not_logical)(int64_t n1)
{
    int64_t rv;

    fs_push(n1);

    _bang();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, !n1);
}

TEST("_bang", "Test terminfo format string logical 'not' operator")
{
    src_parse_init();
    our(test_not_logical)(0x55);
    our(test_not_logical)(0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_xor)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _caret();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 ^ n2));
}

TEST("_caret", "Test terminfo format string 'xor' operator")
{
    src_parse_init();
    our(test_xor)(0x55, 0xff);
    our(test_xor)(1, 0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_plus)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _plus();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 + n2));
}

TEST("_plus", "Test terminfo format string '+' operator")
{
    src_parse_init();
    our(test_plus)(0x55, 0xff);
    our(test_plus)(1, 0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_minus)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _minus();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 - n2));
}

TEST("_minus", "Test terminfo format string '-' operator")
{
    src_parse_init();
    our(test_minus)(0x55, 0xff);
    our(test_minus)(1, 0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_star)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _star();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 * n2));
}

TEST("_star", "Test terminfo format string '*' operator")
{
    src_parse_init();
    our(test_star)(0x55, 0xff);
    our(test_star)(1, 0);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_slash)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _slash();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 / n2));
}

TEST("_slash", "Test terminfo format string '/' operator")
{
    src_parse_init();
    our(test_slash)(0x55, 0x23);
    our(test_slash)(10, 2);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_mod)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _mod();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 % n2));
}

TEST("_mod", "Test terminfo format string 'mod' operator")
{
    src_parse_init();
    our(test_mod)(0x55, 0x23);
    our(test_mod)(10, 2);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_equals)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _equals();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 == n2));
}

TEST("_mod", "Test terminfo format string '=' operator")
{
    src_parse_init();
    our(test_mod)(0x55, 0x55);
    our(test_mod)(10, 2);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_greater)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _greater();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 > n2));
}

TEST("_greater", "Test terminfo format string '>' operator")
{
    src_parse_init();
    our(test_greater)(0x22, 0x55);
    our(test_greater)(10, 2);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_less)(int64_t n1, int64_t n2)
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _less();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 < n2));
}

TEST("_less", "Test terminfo format string '<' operator")
{
    src_parse_init();
    our(test_less)(0x22, 0x55);
    our(test_less)(10, 2);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_tick)(char c)
{
    int16_t prev_num_esc;
    char got;

    f_str = &c;
    prev_num_esc = num_esc;

    _tick();

    got = esc_buff[prev_num_esc];

    ut_assert_op_int64x(got, ==, c);
    f_str = NULL;
}

TEST("_tick", "Test terminfo format string literal insertion")
{
    src_parse_init();
    our(test_tick)('x');
    our(test_tick)('z');
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_parse_i)(int64_t n1, int64_t n2)
{
    params[0] = n1;
    params[1] = n2;

    _i();

    ut_assert_op_int64x(params[0], ==, n1 + 1);
    ut_assert_op_int64x(params[1], ==, n2 + 1);
}

TEST("_i", "Test terminfo format strimg 'i' operator")
{
    src_parse_init();
    our(test_parse_i)(12, 34);
    our(test_parse_i)(56, 78);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_parse_s)(char *s)
{
    int16_t prev_num_esc;

    fs_push((int64_t)s);

    prev_num_esc = num_esc;

    _s();

    ut_assert_strcmp(0, ==, &esc_buff[prev_num_esc], s);
    ut_assert_op_int64x(num_esc, ==, prev_num_esc + strlen(s));
}

TEST("_s", "Test terminfo format string 's' operator")
{
    src_parse_init();
    our(test_parse_s)("The cat in the hat");
    our(test_parse_s)("To stupidity and beyond");
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_parse_l)(char *s)
{
    int rv;

    fs_push((int64_t)s);

    _l();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, strlen(s));
}

TEST("_l", "Test terminfo format string 'l' operator")
{
    src_parse_init();
    our(test_parse_l)("The cat in the hat");
    our(test_parse_l)("To stupidity and beyond");
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(get_var_addr)(void)
{
    int64_t *p;

    char *var1 = "a";
    char *var2 = "Z";


    f_str = var1;
    p = get_var_addr();
    ut_assert_op_void_pt(p, ==, &atoz[0]);

    f_str = var2;
    p = get_var_addr();
    ut_assert_op_void_pt(p, ==, &AtoZ[25]);
}

TEST("get_var_addr", "Test terminfo format string var addr calc")
{
    src_parse_init();
    our(get_var_addr)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_P)(void)
{
    char *var1 = "a";
    char *var2 = "Z";

    atoz[0] = 0;
    fs_push(0x1234);
    f_str = var1;
    _P();
    ut_assert_op_int64x(atoz[0], ==, 0x1234);

    AtoZ[25] = 0;
    src_parse_init();
    fs_push(0x5678);
    f_str = var2;
    _P();
    ut_assert_op_int64x(AtoZ[25], ==, 0x5678);
}

TEST("_P", "Test terminfo format string 'P' operator")
{
    src_parse_init();
    our(test_P)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_g)(void)
{
    int64_t rv;

    char *var1 = "a";
    char *var2 = "Z";

    atoz[0]  = 0x1234;
    AtoZ[25] = 0x5678;

    f_str = var1;
    _g();
    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, 0x1234);

    f_str = var2;
    _g();
    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, 0x5678);
}

TEST("_g", "Test terminfo format string 'g' operator")
{
    src_parse_init();
    our(test_g)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_brace)(void)
{
    int64_t rv;

    char *var1 = "1234}";
    char *var2 = "5678}";

    f_str = var1;
    _brace();
    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, 1234);

    f_str = var2;
    _brace();
    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, 5678);
}

TEST("_brace", "Test terminfo parse of format string numbers")
{
    src_parse_init();
    our(test_brace)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_to_cmd)(void)
{
    char *var1 = "abcd%efg";
    char *var2 = "hij%klm";

    f_str = var1;
    to_cmd();
    f_str--;
    ut_assert_op_int8(*f_str, ==, '%');
    ut_assert_op_int64x((int64_t)f_str, ==, (int64_t)&var1[4]);

    f_str = var2;
    to_cmd();
    f_str--;
    ut_assert_op_int8(*f_str, ==, '%');
    ut_assert_op_int64x((int64_t)f_str, ==, (int64_t)&var2[3]);
}

TEST("to_cmd", "Test terminfo parse to next format string '%' char")
{
    src_parse_init();
    our(test_to_cmd)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_t)(void)
{
    char *var1 = "Zaa%;Xxx";
    char *var2 = "Zaa%eXxx";
    char *var3 = "Zaa%?aa%;Aa%;X";
    char rv;

    f_str = var1;
    fs_push(1);
    _t();
    rv = *f_str;
    ut_assert_op_int8(rv, ==, 'Z');

    f_str = var1;
    fs_push(0);
    _t();
    rv = *f_str;
    ut_assert_op_int8(rv, ==, 'X');

    f_str = var2;
    fs_push(0);
    _t();
    rv = *f_str;
    ut_assert_op_int8(rv, ==, 'X');

    f_str = var3;
    fs_push(0);
    _t();
    rv = *f_str;
    ut_assert_op_int8(rv, ==, 'X');
}

TEST("_t", "Test terminfo format string 't' operator")
{
    src_parse_init();
    our(test_t)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_e)(void)
{
    char *var1 = "xxx%;X";
    char *var2 = "xxx%?aaaaa%eZzzzz%;zzzz%;X";
    char rv;

    f_str = var1;
    _e();
    rv = *f_str;
    ut_assert_op_int8(rv, ==, 'X');

    f_str = var2;
    _e();
    rv = *f_str;
    ut_assert_op_int8(rv, ==, 'X');
}

TEST("_e", "Test terminfo format string 'e' operator")
{
    src_parse_init();
    our(test_e)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("_d", "STUBB")
{
    src_parse_init();
    // deferred till mocking is available
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_emit_tos)(char sent)
{
    char got;
    uint16_t prev_num_esc;

    fs_push(sent);

    prev_num_esc = num_esc;
    _c();
    got = esc_buff[prev_num_esc];
    ut_assert_op_int8(got, ==, sent);
}

TEST("_c", "Test terminfo format string 'c' operator")
{
    src_parse_init();
    our(test_emit_tos)('x');
    our(test_emit_tos)('z');
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_p)(void)
{
    int64_t rv;

    char *var1 = "1";
    char *var2 = "7";

    params[0] = 0x1234;
    params[6] = 0x5678;

    f_str = var1;
    _p();
    rv = fs_pop();
    ut_assert_op_int64x(rv, ==, 0x1234);

    f_str = var2;
    _p();
    rv = fs_pop();
    ut_assert_op_int64x(rv, ==, 0x5678);
}

TEST("_p", "Test terminfo format string 'p' operator")
{
    src_parse_init();
    our(test_p)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(test_next_c)(void)
{
    char rv;
    char *var1 = "abcde";
    char *var2 = "2b000";

    digits = 0;

    f_str = var1;
    rv = next_c();
    ut_assert_op_int8(rv, ==, 'a');
    ut_assert_op_int8(digits, ==, 1);

    f_str = var2;
    rv = next_c();
    ut_assert_op_int8(rv, ==, 'b');
    ut_assert_op_int8(digits, ==, 2);
}

TEST("next_c", "Test read of next char from format string")
{
    src_parse_init();
    our(test_next_c)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("parse_format", "STUBB")
{
    src_parse_init();
    // deferred till mocking is available
    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("format", "STUBB")
{
    src_parse_init();
    // deferred till mocking is available
    src_parse_teardown();
}

// =======================================================================

