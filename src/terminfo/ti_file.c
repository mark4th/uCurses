// tifle.c   - uCurses terminfo file loading
// -----------------------------------------------------------------------

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_parse.h"

// -----------------------------------------------------------------------

typedef struct
{
    uint16_t ti_magic;
    uint16_t ti_names;
    uint16_t ti_bool;
    uint16_t ti_numbers;
    uint16_t ti_strings;
    uint16_t ti_stsize;
} ti_hdr_t;

// -----------------------------------------------------------------------

ti_file_t *ti_file;

// -----------------------------------------------------------------------
// terminfo can also be in ~/terminfo but who puts it there?

char *paths[] =
{
    "/usr/share/terminfo/", // the RIGHT place for terminfo files
    "/lib/terminfo/",       // the stupidest place to put it
    "/etc/terminfo/",       // better than /lib debian idiocy
};

// -----------------------------------------------------------------------

void free_info(void)
{
    munmap(ti_file->ti_map, ti_file->ti_size);
    free(ti_file);
}

// -----------------------------------------------------------------------
// try load terminfo file from one of the above paths

#define MAX_PATH (128)     // aughta be enough for anyone

static bool try_path(int i, const char *env_term)
{
    int len;
    int fd;
    char path[MAX_PATH] = { 0 };
    struct stat st;

    len = strlen(paths[i]);
    strncpy(path, paths[i], len);
    path[len++] = env_term[0];
    path[len] = '/';
    len = strlen(env_term);
    strncat(path, env_term, len);

    stat(path, &st);
    ti_file->ti_size = st.st_size;

    fd = open((char *)path, O_RDONLY, 0);
    if (fd != -1)
    {
        ti_file->ti_map =  (int8_t *)mmap(NULL, ti_file->ti_size,
            PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);

        return (ti_file->ti_map != MAP_FAILED);
    }
    return false;
}

// -----------------------------------------------------------------------

static void map_tifile(void)
{
    int i;
    const char *env_term;

    ti_file = calloc(1, sizeof(*ti_file));
    assert(ti_file);

    env_term = getenv("TERM");
    if (env_term == NULL)
    {
        // this may be incorrect
        printf("No TERM variable set\r\n");
        exit(0);
    }

    for (i = 0; i < 3; i++)
    {
        if (try_path(i, env_term))
        {
            return;
        }
    }

    free(ti_file);
    printf("No Terminfo File found for %s\n", env_term);
    exit(1);
}

// -----------------------------------------------------------------------

static void is_valid(void)
{
    int16_t magic;

    magic = ((ti_hdr_t *)ti_file->ti_map)->ti_magic;

    if ((magic != 0x011a) && (magic != 0x021e))
    {
        free_info();
        printf("Terminfo: Bad Magic\n");
        exit(1);
    }
    // shift value used to calculate offset to end of tables
    // in init_info() above
    ti_file->wide = (magic == 0x021e) ? 2 : 1;
}

// -----------------------------------------------------------------------

void alloc_info()
{
    ti_hdr_t *p;

    int16_t offset;

    map_tifile();           // memory map correct terminfo file
    is_valid();             // verify its magic is correct

    offset = sizeof(ti_hdr_t);

    p = (ti_hdr_t *)ti_file->ti_map;

    // set pointer to names section
    ti_file->ti_names = (char *)&ti_file->ti_map[offset];
    offset += p->ti_names;

    // set pointer to bool section (align if odd length)
    ti_file->ti_bool = &ti_file->ti_map[offset];
    offset += p->ti_bool;
    offset += (offset & 1);

    // set pointer to numbers section which can have 16 or 32 it items
    ti_file->ti_numbers = (int16_t *)&ti_file->ti_map[offset];
    offset += (p->ti_numbers << ti_file->wide);

    // set pointer to strings section which is an array of 16 bit offstts
    // into the table section (below)
    ti_file->ti_strings = (int16_t *)&ti_file->ti_map[offset];
    offset += (p->ti_strings << 1);

    // set address of table section which is a table of escape sequence
    // format strings
    ti_file->ti_table = (char *)&ti_file->ti_map[offset];
}

// =======================================================================
