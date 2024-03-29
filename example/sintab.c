// Sin and Cos tables based on 512 degree circle.
// -----------------------------------------------------------------------

#include <stdint.h>

static int16_t dots_sin_tab[512] =
{
  0x0000, 0x00c9, 0x0192, 0x025b, 0x0323, 0x03ec, 0x04b5, 0x057d,
  0x0645, 0x070d, 0x07d5, 0x089c, 0x0964, 0x0a2a, 0x0af1, 0x0bb6,
  0x0c7c, 0x0d41, 0x0e05, 0x0ec9, 0x0f8c, 0x104f, 0x1111, 0x11d3,
  0x1294, 0x1354, 0x1413, 0x14d1, 0x158f, 0x164c, 0x1708, 0x17c3,
  0x187d, 0x1937, 0x19ef, 0x1aa6, 0x1b5d, 0x1c12, 0x1cc6, 0x1d79,
  0x1e2b, 0x1edc, 0x1f8b, 0x2039, 0x20e7, 0x2192, 0x223d, 0x22e6,
  0x238e, 0x2434, 0x24da, 0x257d, 0x261f, 0x26c0, 0x275f, 0x27fd,
  0x2899, 0x2934, 0x29cd, 0x2a65, 0x2afa, 0x2b8e, 0x2c21, 0x2cb2,
  0x2d41, 0x2dce, 0x2e5a, 0x2ee3, 0x2f6b, 0x2ff1, 0x3076, 0x30f8,
  0x3179, 0x31f7, 0x3274, 0x32ee, 0x3367, 0x33de, 0x3453, 0x34c6,
  0x3536, 0x35a5, 0x3612, 0x367c, 0x36e5, 0x374b, 0x37af, 0x3811,
  0x3871, 0x38cf, 0x392a, 0x3983, 0x39da, 0x3a2f, 0x3a82, 0x3ad2,
  0x3b20, 0x3b6c, 0x3bb6, 0x3bfd, 0x3c42, 0x3c84, 0x3cc5, 0x3d02,
  0x3d3e, 0x3d77, 0x3dae, 0x3de2, 0x3e14, 0x3e44, 0x3e71, 0x3e9c,
  0x3ec5, 0x3eeb, 0x3f0e, 0x3f2f, 0x3f4e, 0x3f6a, 0x3f84, 0x3f9c,
  0x3fb1, 0x3fc3, 0x3fd3, 0x3fe1, 0x3fec, 0x3ff4, 0x3ffb, 0x3ffe,

  0x3fff, 0x3ffe, 0x3ffb, 0x3ff4, 0x3fec, 0x3fe1, 0x3fd3, 0x3fc3,
  0x3fb1, 0x3f9c, 0x3f84, 0x3f6a, 0x3f4e, 0x3f2f, 0x3f0e, 0x3eeb,
  0x3ec5, 0x3e9c, 0x3e71, 0x3e44, 0x3e14, 0x3de2, 0x3dae, 0x3d77,
  0x3d3e, 0x3d02, 0x3cc5, 0x3c84, 0x3c42, 0x3bfd, 0x3bb6, 0x3b6c,
  0x3b20, 0x3ad2, 0x3a82, 0x3a2f, 0x39da, 0x3983, 0x392a, 0x38cf,
  0x3871, 0x3811, 0x37af, 0x374b, 0x36e5, 0x367c, 0x3612, 0x35a5,
  0x3536, 0x34c6, 0x3453, 0x33de, 0x3367, 0x32ee, 0x3274, 0x31f7,
  0x3179, 0x30f8, 0x3076, 0x2ff1, 0x2f6b, 0x2ee3, 0x2e5a, 0x2dce,
  0x2d41, 0x2cb2, 0x2c21, 0x2b8e, 0x2afa, 0x2a65, 0x29cd, 0x2934,
  0x2899, 0x27fd, 0x275f, 0x26c0, 0x261f, 0x257d, 0x24da, 0x2434,
  0x238e, 0x22e6, 0x223d, 0x2192, 0x20e7, 0x2039, 0x1f8b, 0x1edc,
  0x1e2b, 0x1d79, 0x1cc6, 0x1c12, 0x1b5d, 0x1aa6, 0x19ef, 0x1937,
  0x187d, 0x17c3, 0x1708, 0x164c, 0x158f, 0x14d1, 0x1413, 0x1354,
  0x1294, 0x11d3, 0x1111, 0x104f, 0x0f8c, 0x0ec9, 0x0e05, 0x0d41,
  0x0c7c, 0x0bb6, 0x0af1, 0x0a2a, 0x0964, 0x089c, 0x07d5, 0x070d,
  0x0645, 0x057d, 0x04b5, 0x03ec, 0x0323, 0x025b, 0x0192, 0x00c9,

  0x0000, 0xff37, 0xfe6e, 0xfda5, 0xfcdd, 0xfc14, 0xfb4b, 0xfa83,
  0xf9bb, 0xf8f3, 0xf82b, 0xf764, 0xf69c, 0xf5d6, 0xf50f, 0xf44a,
  0xf384, 0xf2bf, 0xf1fb, 0xf137, 0xf074, 0xefb1, 0xeeef, 0xee2d,
  0xed6c, 0xecac, 0xebed, 0xeb2f, 0xea71, 0xe9b4, 0xe8f8, 0xe83d,
  0xe783, 0xe6c9, 0xe611, 0xe55a, 0xe4a3, 0xe3ee, 0xe33a, 0xe287,
  0xe1d5, 0xe124, 0xe075, 0xdfc7, 0xdf19, 0xde6e, 0xddc3, 0xdd1a,
  0xdc72, 0xdbcc, 0xdb26, 0xda83, 0xd9e1, 0xd940, 0xd8a1, 0xd803,
  0xd767, 0xd6cc, 0xd633, 0xd59b, 0xd506, 0xd472, 0xd3df, 0xd34e,
  0xd2bf, 0xd232, 0xd1a6, 0xd11d, 0xd095, 0xd00f, 0xcf8a, 0xcf08,
  0xce87, 0xce09, 0xcd8c, 0xcd12, 0xcc99, 0xcc22, 0xcbad, 0xcb3a,
  0xcaca, 0xca5b, 0xc9ee, 0xc984, 0xc91b, 0xc8b5, 0xc851, 0xc7ef,
  0xc78f, 0xc731, 0xc6d6, 0xc67d, 0xc626, 0xc5d1, 0xc57e, 0xc52e,
  0xc4e0, 0xc494, 0xc44a, 0xc403, 0xc3be, 0xc37c, 0xc33b, 0xc2fe,
  0xc2c2, 0xc289, 0xc252, 0xc21e, 0xc1ec, 0xc1bc, 0xc18f, 0xc164,
  0xc13b, 0xc115, 0xc0f2, 0xc0d1, 0xc0b2, 0xc096, 0xc07c, 0xc064,
  0xc04f, 0xc03d, 0xc02d, 0xc01f, 0xc014, 0xc00c, 0xc005, 0xc002,

  0xc001, 0xc002, 0xc005, 0xc00c, 0xc014, 0xc01f, 0xc02d, 0xc03d,
  0xc04f, 0xc064, 0xc07c, 0xc096, 0xc0b2, 0xc0d1, 0xc0f2, 0xc115,
  0xc13b, 0xc164, 0xc18f, 0xc1bc, 0xc1ec, 0xc21e, 0xc252, 0xc289,
  0xc2c2, 0xc2fe, 0xc33b, 0xc37c, 0xc3be, 0xc403, 0xc44a, 0xc494,
  0xc4e0, 0xc52e, 0xc57e, 0xc5d1, 0xc626, 0xc67d, 0xc6d6, 0xc731,
  0xc78f, 0xc7ef, 0xc851, 0xc8b5, 0xc91b, 0xc984, 0xc9ee, 0xca5b,
  0xcaca, 0xcb3a, 0xcbad, 0xcc22, 0xcc99, 0xcd12, 0xcd8c, 0xce09,
  0xce88, 0xcf08, 0xcf8a, 0xd00f, 0xd095, 0xd11d, 0xd1a6, 0xd232,
  0xd2bf, 0xd34e, 0xd3df, 0xd472, 0xd506, 0xd59b, 0xd633, 0xd6cc,
  0xd767, 0xd803, 0xd8a1, 0xd940, 0xd9e1, 0xda83, 0xdb26, 0xdbcc,
  0xdc72, 0xdd1a, 0xddc3, 0xde6e, 0xdf19, 0xdfc7, 0xe075, 0xe124,
  0xe1d5, 0xe287, 0xe33a, 0xe3ee, 0xe4a3, 0xe55a, 0xe611, 0xe6c9,
  0xe783, 0xe83d, 0xe8f8, 0xe9b4, 0xea71, 0xeb2f, 0xebed, 0xecac,
  0xed6c, 0xee2d, 0xeeef, 0xefb1, 0xf074, 0xf137, 0xf1fb, 0xf2bf,
  0xf384, 0xf44a, 0xf50f, 0xf5d6, 0xf69c, 0xf764, 0xf82b, 0xf8f3,
  0xf9bb, 0xfa83, 0xfb4b, 0xfc14, 0xfcdd, 0xfda5, 0xfe6e, 0xff37
};

int dots_sin(int16_t angle)
{
    int sin = dots_sin_tab[angle];
    return sin;
}

int dots_cos(int16_t angle)
{
    int cos;
    angle += 128;
    angle &= 0x1ff;
    cos = dots_sin_tab[angle];
    return cos;
}

// =======================================================================
