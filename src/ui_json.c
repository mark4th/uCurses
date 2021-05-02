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

char banner[] = "\nlibuCurses v0.0.1\n";

FILE *fp;

// -----------------------------------------------------------------------

char *next_arg(void)
{
    int i;
    static char buff[64];
    char c;
    i = 0;

    while(fread(&c, 1, sizeof(char), fp) == 1)
    {
        if((i == 63) || (c == '\0') ||
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
  printf("// -----------------------------------"
    "------------------------------------\n\n");
}

// -----------------------------------------------------------------------

void hash_file(const char *p)
{
    char path[64];
    char *s;
    uint32_t hash;
    char line_buff[64];
    strcpy(path, p);
    fclose(fp);
    fp = fopen(path, "r");

    if(fp == NULL)
    {
        printf("Cannot open file: %s", path);
        _exit(1);
    }
    printf("\n");
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

    separator();

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
        "}\n\n", path);
    separator();
}

// -----------------------------------------------------------------------

static void help(void)
{
    printf("./libuCurses.so [args]\n\n");
    printf("--help          Get Help (this page)\n");
    printf("--secret        Get Secrets (you no look!)\n");
    printf("file            Hash File (as follows)\n\n");

    printf(
    "The specifid file should contain a list of tags associated\n"
    "with menu functions within your application.\n\n"

    "You might have a menu function called get_foo() and wish\n"
    "to call this function when a specific menu item as described\n"
    "in your json file is selected.\n\n"

    "Invoking the library with a file containing tags for every\n"
    "menu function will cause it to produce the C code you would\n"
    "need to add to your application sources in order for the json\n"
    "parser to associate menu item tags with application functions.\n\n"

    "For example, your json could have an entry such as\n\n"

    "\t\"menu-item\" :\n"
    "\t{\n"
    "\t    \"name\"   : \"Get Foo\",\n"
    "\t    \"vector\" : \"get_foo\"\n"
    "\t}\n\n"

    "and the json parser would replace the vector name string with\n"
    "the C runtime address of the associated function.  The vector\n"
    "string must be identical to the tag specified in the file passed\n"
    "to the library.\n\n");
}

// -----------------------------------------------------------------------

static void process_args(void)
{
    char *s;
    uint32_t hash;

    next_arg();   // skip argv[0]

    while((s = next_arg()) != NULL)
    {
        hash = fnv_hash(s);

        (hash == 0x24a78f02)
            ? help()
            : hash_file(s);
    }

    fclose(fp);
}

// -----------------------------------------------------------------------

void entry(void)
{
    printf("%s\n", banner);

    fp = fopen("/proc/self/cmdline", "r");

    if(fp != NULL)
    {
        process_args();
    }

    _exit(0);
}

// =======================================================================
