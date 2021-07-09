// assert.h  - extensions to unit test framework (as needed)
// -----------------------------------------------------------------------

#define ut_assert_op_int64(_left, op, _right) \
    ut_assert_op(_left, op, _right, const int64_t, "%" PRI64)

#define ut_assert_op_int64x(_left, op, _right) \
    ut_assert_op(_left, op, _right, const int64_t, "%" PRIx64)

#define ut_assert_op_int16(_left, op, _right) \
    ut_assert_op(_left, op, _right, const int16_t, "%" PRI16)

#define ut_assert_op_int16x(_left, op, _right) \
    ut_assert_op(_left, op, _right, const int16_t, "%" PRIx16)

// -----------------------------------------------------------------------
// they didnt seem to want to give me a fomat string for uin16_t in hex

#define ut_assert_op_uint16(_left, op, _right) \
    ut_assert_op(_left, op, _right, const uint16_t, "%" PRIu16)

#define ut_assert_op_int8(_left, op, _right) \
    ut_assert_op(_left, op, _right, const int8_t, "%" PRIx8)

// =======================================================================
