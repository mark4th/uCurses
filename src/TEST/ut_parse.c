// parse.c -- unit tests
// -----------------------------------------------------------------------

#include <libchordtest/test.h>
#include "h/assert.h"
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
#define frame check_c_emit

// ------------------------------------------------------------------------

void our(check_c_emit)(char sent)
{
    uint16_t prev_num_esc;
    char got;

    prev_num_esc = num_esc;

    c_emit(sent);

    got = esc_buff[prev_num_esc];

    ut_assert_op_int16x(num_esc, ==, prev_num_esc + 1);
    ut_assert_op_int16x(got, ==, sent);
}

TEST("c_emit", "check for add to esc_buff")
{
    src_parse_init();
    our(check_c_emit)('*');
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_fs_push)(int64_t sent)
{
    int8_t prev_fsp;
    int64_t got;

    prev_fsp = fsp;

    fs_push(sent);

    got = fstack[prev_fsp];

    ut_assert_op_int8(fsp, ==, prev_fsp + 1);
    ut_assert_op_int64x(got, ==, sent);
}

TEST("fs_push", "check format string stack push")
{
    src_parse_init();
    our(check_fs_push)(0x12);
    our(check_fs_push)(0x34);

    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_fs_pop)(int64_t sent)
{
    int8_t prev_fsp;
    int64_t got;

    fs_push(sent);

    prev_fsp = fsp;
    got = fs_pop();

    ut_assert_op_int8(fsp, ==, prev_fsp - 1);
    ut_assert_op_int64x(got, ==, sent);
}

TEST("fs_push", "check format string stack pop")
{
    src_parse_init();
    our(check_fs_push)(0x12);
    our(check_fs_push)(0x34);
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_percent)(void)
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

TEST("check_percent", "verify addition of '%' char to escape buffer")
{
    src_parse_init();
    our(check_percent)();
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_and(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _and();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 & n2));
}

TEST("_and", "check format string 'and' operator")
{
    src_parse_init();
    our(check_and(0x55, 0xff));
    our(check_and(3, 2));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_and_logical(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _andl();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 && n2));
}

TEST("_andl", "check format string logical 'and' operator")
{
    src_parse_init();
    our(check_and_logical(0x55, 0xff));
    our(check_and_logical(1, 0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_or(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _or();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 | n2));
}

TEST("_or", "check format string 'or' operator")
{
    src_parse_init();
    our(check_or(0x55, 0xff));
    our(check_or(1, 0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_or_logical(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _orl();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 || n2));
}

TEST("_orl", "check format string logical 'or' operator")
{
    src_parse_init();
    our(check_or_logical(0x55, 0xff));
    our(check_or_logical(1, 0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_not(int64_t n1))
{
    int64_t rv;

    fs_push(n1);

    _tilde();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, ~n1);
}

TEST("_tilde", "check format string 'not' operator")
{
    src_parse_init();
    our(check_not(0x55));
    our(check_not(0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_not_logical(int64_t n1))
{
    int64_t rv;

    fs_push(n1);

    _bang();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, !n1);
}

TEST("_bang", "check format string logical 'not' operator")
{
    src_parse_init();
    our(check_not_logical(0x55));
    our(check_not_logical(0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_xor(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _caret();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 ^ n2));
}

TEST("_caret", "check format string 'xor' operator")
{
    src_parse_init();
    our(check_xor(0x55, 0xff));
    our(check_xor(1, 0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_plus(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _plus();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 + n2));
}

TEST("_plus", "check format string '+' operator")
{
    src_parse_init();
    our(check_plus(0x55, 0xff));
    our(check_plus(1, 0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_minus(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _minus();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 - n2));
}

TEST("_minus", "check format string '-' operator")
{
    src_parse_init();
    our(check_minus(0x55, 0xff));
    our(check_minus(1, 0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_star(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _star();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 * n2));
}

TEST("_star", "check format string '*' operator")
{
    src_parse_init();
    our(check_star(0x55, 0xff));
    our(check_star(1, 0));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_slash(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _slash();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 / n2));
}

TEST("_slash", "check format string '/' operator")
{
    src_parse_init();
    our(check_slash(0x55, 0x23));
    our(check_slash(10, 2));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_mod(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _mod();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 % n2));
}

TEST("_mod", "check format string 'mod' operator")
{
    src_parse_init();
    our(check_mod(0x55, 0x23));
    our(check_mod(10, 2));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_equals(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _equals();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 == n2));
}

TEST("_mod", "check format string '=' operator")
{
    src_parse_init();
    our(check_mod(0x55, 0x55));
    our(check_mod(10, 2));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_greater(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _greater();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 > n2));
}

TEST("_greater", "check format string '>' operator")
{
    src_parse_init();
    our(check_greater(0x22, 0x55));
    our(check_greater(10, 2));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_less(int64_t n1, int64_t n2))
{
    int64_t rv;

    fs_push(n1);
    fs_push(n2);

    _less();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, (n1 < n2));
}

TEST("_less", "check format string '<' operator")
{
    src_parse_init();
    our(check_less(0x22, 0x55));
    our(check_less(10, 2));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_tick(char c))
{
    int16_t prev_num_esc;
    char got;

    f_str = &c;
    prev_num_esc = num_esc;

    _tick();

    got = esc_buff[prev_num_esc];

    ut_assert_op_int64x(got, ==, c);
}

TEST("_tick", "check format string literal insertion")
{
    src_parse_init();
    our(check_tick('x'));
    our(check_tick('z'));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_parse_i(int64_t n1, int64_t n2))
{
    params[0] = n1;
    params[1] = n2;

    _i();

    ut_assert_op_int64x(params[0], ==, n1 + 1);
    ut_assert_op_int64x(params[1], ==, n2 + 1);
}

TEST("_i", "check format strimg 'i' operator")
{
    src_parse_init();
    our(check_parse_i(12, 34));
    our(check_parse_i(56, 78));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_parse_s(char *s))
{
    int16_t prev_num_esc;

    fs_push((int64_t)s);

    prev_num_esc = num_esc;

    _s();

    ut_assert_strcmp(0, ==, &esc_buff[prev_num_esc], s);
    ut_assert_op_int64x(num_esc, ==, prev_num_esc + strlen(s));
}

TEST("_s", "check format string 's' operator")
{
    src_parse_init();
    our(check_parse_s("The cat in the hat"));
    our(check_parse_s("To stupidity and beyond"));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

void our(check_parse_l(char *s))
{
    int rv;

    fs_push((int64_t)s);

    _l();

    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, strlen(s));
}

TEST("_l", "check format string 'l' operator")
{
    src_parse_init();
    our(check_parse_l("The cat in the hat"));
    our(check_parse_l("To stupidity and beyond"));
    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("get_var_addr", "check calc of format string variable address")
{
    int64_t *p;

    char *var1 = "a";
    char *var2 = "Z";

    src_parse_init();

    f_str = var1;
    p = get_var_addr();
    ut_assert_op_void_pt(p, ==, &atoz[0]);

    f_str = var2;
    p = get_var_addr();
    ut_assert_op_void_pt(p, ==, &AtoZ[25]);

    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("_P", "check setting of format string variables")
{
    char *var1 = "a";
    char *var2 = "Z";

    src_parse_init();

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

    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("_P", "check getting of format string variables")
{
    int64_t rv;

    char *var1 = "a";
    char *var2 = "Z";

    src_parse_init();

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

    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("_brace", "check parsing of format string numbers")
{
    int64_t rv;

    char *var1 = "1234}";
    char *var2 = "5678}";

    src_parse_init();

    f_str = var1;
    _brace();
    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, 1234);

    f_str = var2;
    _brace();
    rv = fs_pop();

    ut_assert_op_int64x(rv, ==, 5678);

    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("to_cmd", "verify pparse to next format string '%' char")
{
    char *var1 = "abcd%efg";
    char *var2 = "hij%klm";

    src_parse_init();

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

    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("_t", "verify parsting of terminfo 't' operator")
{
    char *var1 = "Zaa%;Xxx";
    char *var2 = "Zaa%eXxx";
    char *var3 = "Zaa%?aa%;Aa%;X";
    char rv;

    src_parse_init();

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

    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("_e", "Verify terminfo format string 'e' operator")
{
    char *var1 = "xxx%;X";
    char *var2 = "xxx%?aaaaa%eZzzzz%;zzzz%;X";
    char rv;

    src_parse_init();

    f_str = var1;
    _e();
    rv = *f_str;
    ut_assert_op_int8(rv, ==, 'X');

    f_str = var2;
    _e();
    rv = *f_str;
    ut_assert_op_int8(rv, ==, 'X');

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

void our(check_emit_tos)(char sent)
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
    our(check_emit_tos)('x');
    our(check_emit_tos)('z');
    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("_p", "Test terminfo format string 'p' operator")
{
    int64_t rv;

    char *var1 = "1";
    char *var2 = "7";

    src_parse_init();
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

    src_parse_teardown();
}

// -----------------------------------------------------------------------

TEST("next_c", "Test Get of next character from format string")
{
    char rv;
    char *var1 = "abcde";
    char *var2 = "2b000";

    src_parse_init();
    digits = 0;

    f_str = var1;
    rv = next_c();
    ut_assert_op_int8(rv, ==, 'a');
    ut_assert_op_int8(digits, ==, 1);

    f_str = var2;
    rv = next_c();
    ut_assert_op_int8(rv, ==, 'b');
    ut_assert_op_int8(digits, ==, 2);

    src_parse_teardown();
}

// =======================================================================

