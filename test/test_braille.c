// test_braille.c  - tests for UTF-8 braille bitmap conversion
// -----------------------------------------------------------------------

#include "unity/unity.h"
#include "uCurses.h"
#include "uC_window.h"
#include "uC_braille.h"

void setUp(void)    {}
void tearDown(void) {}

// -----------------------------------------------------------------------

static uint8_t reference_pack_cell(uint8_t *d1, uint8_t *d2, uint8_t *d3,
    uint8_t *d4)
{
    uint8_t d0 = 0;

    d0 >>= 1;     d0 |= *d1 & 0x80;     *d1 <<= 1;
    d0 >>= 1;     d0 |= *d2 & 0x80;     *d2 <<= 1;
    d0 >>= 1;     d0 |= *d3 & 0x80;     *d3 <<= 1;
    d0 >>= 1;     d0 |= *d4 & 0x80;     *d4 <<= 1;

    d0 >>= 1;     d0 |= *d1 & 0x80;     *d1 <<= 1;
    d0 >>= 1;     d0 |= *d2 & 0x80;     *d2 <<= 1;
    d0 >>= 1;     d0 |= *d3 & 0x80;     *d3 <<= 1;
    d0 >>= 1;     d0 |= *d4 & 0x80;     *d4 <<= 1;

    return d0;
}

// -----------------------------------------------------------------------

static void reference_braille_8(uint16_t *expected, uint8_t d1, uint8_t d2,
    uint8_t d3, uint8_t d4)
{
    uint8_t z;

    for (z = 0; z < 4; z++)
    {
        expected[z] = uC_braille_xlat(reference_pack_cell(&d1, &d2, &d3,
            &d4));
    }
}

// -----------------------------------------------------------------------

void test_braille_8_matches_shift_packer(void)
{
    uC_window_t win = { .width = 4, .height = 1 };
    uint8_t map[] = { 0x81, 0x42, 0x24, 0x18 };
    uint16_t actual[4] = { 0 };
    uint16_t expected[4] = { 0 };

    reference_braille_8(expected, map[0], map[1], map[2], map[3]);
    uC_braille_8(&win, actual, map, 1);

    TEST_ASSERT_EQUAL_UINT16_ARRAY(expected, actual, 4);
}

// -----------------------------------------------------------------------

static uint8_t reference_1_pack_cell(uint8_t **p0, uint8_t **p1,
    uint8_t **p2, uint8_t **p3)
{
    uint8_t d0;

    d0 = 0;     d0 |= (*(*p0)++ << 7);
    d0 >>= 1;   d0 |= (*(*p1)++ << 7);
    d0 >>= 1;   d0 |= (*(*p2)++ << 7);
    d0 >>= 1;   d0 |= (*(*p3)++ << 7);
    d0 >>= 1;   d0 |= (*(*p0)++ << 7);
    d0 >>= 1;   d0 |= (*(*p1)++ << 7);
    d0 >>= 1;   d0 |= (*(*p2)++ << 7);
    d0 >>= 1;   d0 |= (*(*p3)++ << 7);

    return d0;
}

// -----------------------------------------------------------------------

static void reference_braille_1(uint16_t *expected, uint8_t *map,
    uint16_t width, uint16_t height)
{
    uint16_t x, y;
    uint16_t out_width = width / 2;
    uint8_t *p0;
    uint8_t *p1;
    uint8_t *p2;
    uint8_t *p3;

    p0 = map;

    for (y = 0; y < height / 4; y++)
    {
        p1 = p0 + width;
        p2 = p1 + width;
        p3 = p2 + width;

        for (x = 0; x < out_width; x++)
        {
            expected[(y * out_width) + x] =
                uC_braille_xlat(reference_1_pack_cell(&p0, &p1, &p2, &p3));
        }
        p0 += width * 3;
    }
}

// -----------------------------------------------------------------------

void test_braille_1_matches_shift_packer(void)
{
    uC_window_t win = { .width = 2, .height = 1 };
    uint8_t map[] = {
        1, 0, 0, 1,
        0, 1, 1, 0,
        1, 1, 0, 0,
        0, 0, 1, 1,
    };
    uint16_t actual[2] = { 0 };
    uint16_t expected[2] = { 0 };

    reference_braille_1(expected, map, 4, 4);
    uC_braille_1(&win, actual, map, 4, 4);

    TEST_ASSERT_EQUAL_UINT16_ARRAY(expected, actual, 2);
}

// -----------------------------------------------------------------------

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_braille_8_matches_shift_packer);
    RUN_TEST(test_braille_1_matches_shift_packer);
    return UNITY_END();
}

// =======================================================================
