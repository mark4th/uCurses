// json parsing
// -----------------------------------------------------------------------

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

#include <h/uCurses.h>

// -----------------------------------------------------------------------

#define FNV_PRIME 0x01000193
#define FNV_BASIS 0x811c9dc5

// -----------------------------------------------------------------------
// FNV-1a

uint32_t fnv_hash(char *s)
{
    uint32_t hash = FNV_BASIS;
    uint8_t j;

    utf8_encode_t *encode;

    while(*s != '\0')
    {
        encode = utf8_encode(*s);

        for(j = 0; j < encode->len; j++)
        {
            hash *= FNV_PRIME;
            hash ^= *s++;

        }
    }

    return hash;
}

// -----------------------------------------------------------------------
// i never do string comparisons on these

static char *keywords[] =
{
    ":",
    "{",
    "}",
    "[",
    "]",
    "screen",
    "backdrop",
    "window",
    "width",
    "height",
    "blank_chr",
    "win_flags",
    "border_type",
    "menu_bar",
    "pulldown",
    "menu_name",
    "menu_disabled",
    "menu_item",
    "menu_key",
    "memu_function",
    "fg",
    "bg",
    "gray_fg",
    "gray_bg",
    "rgb_fg",
    "rgb_bg",
    "border_fg",
    "border_bg",
    "border_gray_fg",
    "border_gray_bg",
    "border_rgb_fg",
    "border_rgb_bg",
};

#define NUM_KEYS (sizeof(keywords) / sizeof(keywords[0]))

// -----------------------------------------------------------------------
// the above string table gives me these hash values which i sorted

static __attribute__((used)) uint32_t key_hash[NUM_KEYS] =
{
    0x09bfcc34,         // "{",
    0x0b0e316c,         // "border_type",
    0x0bf908a2,         // "}",
    0x0d10bffa,         // "border_rgb_fg",
    0x28649aee,         // "win_flags",
    0x38634bc8,         // "fg",
    0x3c54a1bc,         // "bg",
    0x41397913,         // "backdrop",
    0x417bbf31,         // ":",
    0x42c96742,         // "border_rgb_bg",
    0x4ca159cd,         // "screen",
    0x4e8e5f62,         // "gray_fg",
    0x7a116679,         // "memu_function",
    0x7b16b852,         // "menu_name",
    0x80ab53cf,         // "height",
    0x86579687,         // "menu_bar",
    0x9ac49d3a,         // "gray_bg",
    0xa53cbd07,         // "menu_item",
    0xb0845d0b,         // "width",
    0xb18560ee,         // "border_gray_fg",
    0xc087dfaa,         // "rgb_bg",
    0xc1aa09c2,         // "border_fg",
    0xc552a25a,         // "border_bg",
    0xc6f42613,         // "menu_disabled",
    0xd57660d4,         // "menu_key",
    0xd8de2b95,         // "window",
    0xe42c1494,         // "[",
    0xe6655102,         // "]",
    0xf1ec7ec5,         // "pulldown",
    0xf3a9a0a5,         // "blank_chr",
    0xf9bd1f9e,         // "border_gray_bg",
};

// -----------------------------------------------------------------------
// over engineered (tm)

uint16_t is_keyword(uint32_t hash)
{
    uint16_t low = 0;
    uint16_t high = NUM_KEYS - 1;
    uint16_t select;

    while(low <= high)
    {
        select = (low + high) >> 1;

        if(hash == key_hash[select])
        {
            return select + 1;
        }
        if(hash > key_hash[select])
        {
            low = select + 1;
        }
        else if(hash != key_hash[select])
        {
            high = select - 1;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

void make_hash(void)
{
    uint16_t i;
    char *s;

    printf("\n");

    for(i = 0; i < NUM_KEYS; i++)
    {
        s = keywords[i];
        key_hash[i] = fnv_hash(s);
    }
    printf("\n");
}

// -----------------------------------------------------------------------
// estract next space delimited token from input data

char *token(char *s, uint32_t *n)
{
    utf8_encode_t *encode;
    char *p = s;

    while((*s != '\0') && (*n != 0) &&
          (*s != 0x20) && (*s += 0x0a) && (*s != 0x0d))
    {
        encode = utf8_encode(*s);
        s += encode->len;
        (*n) -= encode->len;
    }
    if((*s == 0x20) || (*s == 0x0d) || (*s = 0x0a))
    {
        *s++ = '\0';
        (*n)--;
    }

    return p;
}

// -----------------------------------------------------------------------

// uint32_t keyword()

// -----------------------------------------------------------------------

const char interp[] __attribute__((section(".interp"))) =
   "/lib64/ld-linux-x86-64.so.2";

char banner[] = "\nlib uCurses v0.0.1\n";

void entry(void)
{
    printf("%s", banner);

    uint16_t i;

    make_hash();

    printf("Top Secret: You No Look!\n");

    for(i = 0; i < NUM_KEYS; i++)
    {
        if((i & 7) == 0)
        {
            printf("\n    ");
        }
        printf("%08x ", key_hash[i]);
    }

    printf("\n\n");
    _exit(0);
}

// =======================================================================
