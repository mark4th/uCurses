// ui_json.c
// -----------------------------------------------------------------------

// technicaly not really part of the UI build but included for now so I
// can generate the hash values for all the various json keys.
// this should be a totally separate make target really

// -----------------------------------------------------------------------

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

const char interp[] __attribute__((section(".interp"))) =
    "/lib64/ld-linux-x86-64.so.2";

char banner[] = "\nlibuCurses V" PROJECT_VERSION;

static FILE *fp;
static char line_buff[64];

// -----------------------------------------------------------------------

char *next_arg(void)
{
    static char buff[64];

    int i;
    char c;

    i = 0;

    while(fread(&c, 1, sizeof(char), fp) == 1)
    {
        if((i == 63) || (c == '\0') || //
           (c == '\n') || (c == '\r'))
        {
            buff[i] = '\0';
            return buff;
        }
        buff[i++] = c;
    }

    return NULL;
}

// -----------------------------------------------------------------------

static void separator(void)
{
    printf(
        "// -----------------------------------"
        "------------------------------------\n\n");
}

// -----------------------------------------------------------------------

static INLINE void print_cb(char *path)
{
    printf(
        "static opt_t menu_address_cb(uint32_t hash)\n"
        "{\n"
        "    uint16_t i = VCOUNT;\n"
        "    switch_t *s = %s;\n\n"

        "    for(i = 0; i < VCOUNT; i++)\n"
        "    {\n"
        "        if(hash == s->option)\n"
        "        {\n"
        "            return s->vector;\n"
        "        }\n"
        "        s++;\n"
        "    }\n"

        "    return NULL;\n"
        "}\n\n",
        path);
    separator();
}

// -----------------------------------------------------------------------

static INLINE void print_switch(char *path)
{
    char *s;
    uint32_t hash;

    separator();

    printf("static switch_t %s\n{\n", path);

    while((s = next_arg()) != NULL)
    {
        memset(line_buff, 0x20, 63);
        hash = fnv_hash(s);
        snprintf(line_buff, 63, "    { 0x%08x, %s", hash, s);

        line_buff[strlen(line_buff)] = 0x20;

        line_buff[42] = '\0';
        strcat(line_buff, "},");

        printf("%s\n", line_buff);
    }

    printf("};\n\n");

    printf("#define VCOUNT sizeof(%s) / sizeof(%s[0])\n\n", path, path);
}

// -----------------------------------------------------------------------

void hash_file(const char *p)
{
    char path[64];
    strcpy(path, p);
    fclose(fp);
    fp = fopen(path, "r");

    if(fp == NULL)
    {
        printf("Cannot open file: %s", path);
        _exit(1);
    }
    printf("\n");

    print_switch(path);

    separator();
    print_cb(path);
}

// -----------------------------------------------------------------------
// top secret, you no look!!! patent not pending :)

#define SMUSHED
#ifdef SMUSHED
static const uint16_t smushed[] = //
    {                             //
        0x03a6, 0x793e, 0x2989, 0x0abe, 0x32b2, 0x4cb3, 0x7933, 0x3c1e,
        0x5032, 0x1e7e, 0x57c2, 0x785e, 0x23c8, 0x20ac, 0x4000, 0x0000,
        0x0000, 0x001e, 0x38b4, 0x03cf, 0x1590, 0x03c5, 0x5109, 0x4c10,
        0x04e5, 0x78de, 0x08c9, 0x30a0, 0x0000, 0x0000, 0x0000, 0x001e,
        0x3c33, 0x201e, 0x352c, 0x141e, 0x1433, 0x00cf, 0x318f, 0x5e7e,
        0x1bc2, 0x785e, 0x4905, 0x0270, 0x1469, 0x1924, 0x00c9, 0x30a0,
        0x4d0f, 0x5584, 0x006f, 0x3a81, 0x25c0, 0x040c, 0x2674, 0x01e6,
        0x0281, 0x1e60, 0x0673, 0x3c69, 0x0685, 0x13c2, 0x5d34, 0x200d,
        0x15d5, 0x00d5, 0x3874, 0x25ee, 0x4c17, 0x2688, 0x25c0, 0x65f5,

        0x4801, 0x420c, 0x2461, 0x512f, 0x3bc9, 0x785e, 0x0bd3, 0x3ea0,
        0x3527, 0x2280, 0x2036, 0x1401, 0x01a5, 0x3aa0, 0x1aae, 0x0e89,
        0x3dc0, 0x0c2c, 0x30a4, 0x00e5, 0x53d6, 0x19ef, 0x78be, 0x1801,
        0x3880, 0x5d33, 0x23c2, 0x51e0, 0x0c2c, 0x3014, 0x2133, 0x00d5,
        0x3874, 0x25ee, 0x02e8, 0x15c0, 0x0413, 0x40a3, 0x24c9, 0x0c0d,
        0x15d5, 0x0134, 0x15a0, 0x0660, 0x10b3, 0x0e49, 0x08a4, 0x7849,
        0x3819, 0x3eb2, 0x0153, 0x3dc0, 0x192c, 0x1409, 0x4c13, 0x1585,
        0x0e85, 0x13c9, 0x785e, 0x0bd0, 0x3acf, 0x2d2e, 0x1c14, 0x20a0,
        0x3122, 0x4832, 0x6417, 0x2688, 0x0020, 0x192c, 0x1403, 0x3dd4,

        0x052e, 0x25c7, 0x0281, 0x1e60, 0x19f2, 0x00b6, 0x1659, 0x784d,
        0x15d5, 0x00d5, 0x3874, 0x25ee, 0x02e9, 0x3180, 0x0c35, 0x4ca0,
        0x2680, 0x51e0, 0x424f, 0x12a3, 0x1414, 0x20a0, 0x7980, 0x0de4,
        0x1419, 0x3ea0, 0x5df5, 0x309e, 0x09c5, 0x1480, 0x51e0, 0x0484,
        0x028f, 0x032f, 0x5640, 0x0610, 0x3123, 0x0689, 0x3dc0, 0x4df5,
        0x4865, 0x4c09, 0x380f, 0x4885, 0x4806, 0x3e40, 0x5105, 0x0153,
        0x3dde, 0x0a01, 0x4a65, 0x4814, 0x3c01, 0x4e6f, 0x0d21, 0x50a0,
        0x34ae, 0x5409, 0x50ad, 0x0281, 0x1e60, 0x5d34, 0x2001, 0x420c,
        0x2461, 0x512f, 0x3806, 0x55c3, 0x512f, 0x3a7e, 0x27c2, 0x785e,

        0x35f2, 0x00b8, 0x05b0, 0x30be, 0x1c19, 0x3eb2, 0x0153, 0x3dc0,
        0x0df5, 0x3080, 0x2036, 0x1401, 0x3805, 0x3a92, 0x6413, 0x5468,
        0x0033, 0x785e, 0x0bc1, 0x788d, 0x15d5, 0x7909, 0x50ad, 0x7880,
        0x797e, 0x0bc1, 0x6fc2, 0x7820, 0x0000, 0x788e, 0x05a5, 0x7880,
        0x001e, 0x2c1e, 0x13ce, 0x1680, 0x79af, 0x3fc4, 0x78fe, 0x0bc1,
        0x0000, 0x03c4, 0x58a3, 0x51f2, 0x7880, 0x7960, 0x7887, 0x169e,
        0x58cf, 0x3fc4, 0x785e, 0x07be, 0x0bc2, 0x05c4, 0x0288, 0x140a,
        0x4dee, 0x0201, 0x4a65, 0x4817, 0x3eac, 0x1012, 0x160c, 0x0465,
        0x0288, 0x1416, 0x1474, 0x3e40, 0x382d, 0x1413, 0x5249, 0x38e0,

        0x5d34, 0x23c2, 0x5105, 0x03cc, 0x0255, 0x3a89, 0x34a0, 0x0484,
        0x48b3, 0x4c0f, 0x1814, 0x20a0, 0x0673, 0x3c69, 0x0685, 0x1006,
        0x55c3, 0x512f, 0x3bc9, 0x001e, 0x4905, 0x02c5, 0x0e8f, 0x4bc2,
        0x4e92, 0x25c7, 0x01b5, 0x4e80, 0x08a0, 0x2485, 0x3a89, 0x0c2c,
        0x028f, 0x0288, 0x1414, 0x04e0, 0x4e05, 0x0d26, 0x24a4, 0x012e,
        0x0288, 0x1406, 0x2585, 0x0201, 0x4e65, 0x13c2, 0x51e0, 0x5105,
        0x0189, 0x0a41, 0x4b3e, 0x27c2, 0x7840
    };

// -----------------------------------------------------------------------
// smush decompression routines!

static int8_t read_c1(void)
{
    static int i = 1;
    static int q = 0;
    static int16_t blob;

    int8_t c1;
    if(q == 0)
    {
        blob = smushed[i++];
        q = 3;
    }
    c1 = (blob >> 10);
    blob <<= 5;
    blob &= 0x7fff;
    q--;
    return c1;
}

// -----------------------------------------------------------------------

static void help(void)
{
    int8_t c1;
    uint16_t len;

    const uint8_t xlat[] = //
        {                  //
          0x00, 0x09, 0x0a, 0x21, 0x22, 0x28, 0x29, 0x2c,
          0x2d, 0x2e, 0x2f, 0x3a, 0x43, 0x46, 0x47, 0x48,
          0x49, 0x53, 0x54, 0x59, 0x5b, 0x5d, 0x5f
        };

    len = smushed[0];

    do
    {
        c1 = read_c1();
        if(c1 == 0x1e)
        {
            c1 = read_c1();
            c1 = xlat[c1];
        }
        else
        {
            c1 = (c1 == 0) ? 0x20 : c1 + 0x60;
        }
        printf("%c", c1);
        len--;
    } while(len != 0);
}
#else
static const char message[] = //
    {                         //
        "./libuCurses.so [args]\n\n"
        "--help          Get Help (this page)\n"
        "file            Hash File (as follows)\n\n"

        "The specifid file should contain a list of tags associated\n"
        "with menu functions within your application.\n\n"

        "You might have a menu function called get_foo() and wish\n"
        "to call this function when a specific menu item as described\n"
        "in your json file is selected.\n\n"

        "Invoking the library with a file containing tags for every\n"
        "menu function will cause it to produce the C code you would\n"
        "need to add to your application sources in order for the json\n"
        "parser to associate menu item tags with application "
        "functions.\n\n"

        "For example, your json could have an entry such as\n\n"

        "\t\"menu-item\" :\n"
        "\t{\n"
        "\t    \"name\"   : \"Get Foo\",\n"
        "\t    \"vector\" : \"get_foo\"\n"
        "\t}\n\n"

        "and the json parser would replace the vector name string with\n"
        "the C runtime address of the associated function.  The vector\n"
        "string must be identical to the tag specified in the file "
        "passed\n"
        "to the library.\n\n"
    };
static void help(void)
{
    printf("%s", message);
}
#endif

// ------------------------------------------------------------------------
// the smush compression method.  copyright (c) 2021 MM :)

// int count = 0;
// uint16_t blob;
// int nblob;

// ------------------------------------------------------------------------

// void emit(uint16_t c)
// {
//     printf("0x%04x, ", c);
//     count++;
//
//     if((count % 8) == 0)
//     {
//         printf("\n");
//     }
// }

// ------------------------------------------------------------------------

// void smush(uint8_t c)
// {
//     blob <<= 5;
//     blob += c;
//     nblob++;
//
//     if(nblob == 3)
//     {
//         emit(blob);
//         blob = 0;
//         nblob = 0;
//     }
// }

// -----------------------------------------------------------------------

// void smusher(uint8_t c)
// {
//     int i;
//
//     uint8_t xlat[] =
//     {
//         0x00, 0x09, 0x0a, 0x21, 0x22, 0x28, 0x29, 0x2c,
//         0x2d, 0x2e, 0x2f, 0x3a, 0x43, 0x46, 0x47, 0x48,
//         0x49, 0x53, 0x54, 0x59, 0x5b, 0x5d, 0x5f
//     };
//
//     if(c == 0x20)
//     {
//         smush(0);
//         return;
//     }
//
//     for(i = 0; i < sizeof(xlat); i++)
//     {
//         if(c == xlat[i])
//         {
//             smush(0x1e);
//             smush(i);
//             return;
//         }
//     }
//     exit(1);
// }

// -----------------------------------------------------------------------

// void smush_loop(void)
// {
//     printf("0x%04x,\n", (unsigned int)strlen(text));
//     int count = 0;

//     for(i = 0; i < strlen(message); i++)
//     {
//         c = message[i];
//         if(c < 0x61)
//         {
//             smusher(c);
//         }
//         else
//         {
//             c -= 0x60;
//             smush(c);
//         }
//     }

//     while(nblob != 0)
//     {
//         smush(0);
//     }
//     printf("\n");
// }

// -----------------------------------------------------------------------

static void process_args(void)
{
    char *s;
    int32_t hash;

    next_arg(); // skip argv[0]

    while((s = next_arg()) != NULL)
    {
        hash = fnv_hash(s);

        (hash == 0x24a78f02) ? help() : hash_file(s);
    }

    fclose(fp);
}

// -----------------------------------------------------------------------
// entry point when executing .so file directly

void entry(void)
{
    printf("%s\n\n", banner);
    fp = fopen("/proc/self/cmdline", "r");

    if(fp != NULL)
    {
        process_args();
    }

    _exit(0);
}

// =======================================================================
