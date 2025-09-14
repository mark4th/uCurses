// uC_tifle.c   - uCurses terminfo file loading
// -----------------------------------------------------------------------

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_terminfo.h"
#include "uC_parse.h"
#include "uC_utils.h"
#include "uC_alloc.h"

// -----------------------------------------------------------------------

extern ti_vars_t *ti_vars;

#define MAX_PATH (128)     // aughta be enough for anyone

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
// terminfo can also be in ~/terminfo but who puts it there?

char *paths[] =
{
    "/usr/share/terminfo/", // the RIGHT place for terminfo files
    "/lib/terminfo/",       // the stupidest place to put it
    "/etc/terminfo/",       // better than /lib debian idiocy
};

// -----------------------------------------------------------------------
// try load terminfo file from one of the above paths

static bool try_path(int i, const char *env_term)
{
    int len;
    int fd;
    char path[MAX_PATH] = { 0 };
    struct stat st;
    size_t n;

    len = strlen(paths[i]);
    strncpy(path, paths[i], MAX_PATH);
    path[len++] = env_term[0];
    path[len] = '/';

    if (strlen(path) + strlen(env_term) > MAX_PATH)
    {
        return false;
    }
    strcat(path, env_term);

    fd = stat(path, &st);   // is an int so im borrowing it

    if (fd != -1)
    {
        ti_vars->ti_file.ti_size = st.st_size;
        ti_vars->ti_file.ti_map  = uC_alloc(uC_MEM_ZONE_DEFAULT,
            ti_vars->ti_file.ti_size);

        uC_ASSERT(ti_vars->ti_file.ti_map != NULL, "Out of Memory");

        fd = open((char *)path, O_RDONLY, 0);
        if (fd != -1)
        {
            n = read(fd, ti_vars->ti_file.ti_map, ti_vars->ti_file.ti_size);
            close(fd);
            return (n == (size_t)ti_vars->ti_file.ti_size);
        }
        uC_free(uC_MEM_ZONE_DEFAULT, ti_vars->ti_file.ti_map);
        ti_vars->ti_file.ti_size = 0;
        ti_vars->ti_file.ti_map  = 0;
    }

    return false;
}

// -----------------------------------------------------------------------

static void map_tifile(void)
{
    int i;
    const char *env_term;

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

    printf("No Terminfo File found for %s\n", env_term);
    exit(1);
}

// -----------------------------------------------------------------------

static void is_valid(void)
{
    int16_t magic;

    magic = ((ti_hdr_t *)ti_vars->ti_file.ti_map)->ti_magic;

    if ((magic != 0x011a) && (magic != 0x021e))
    {
        printf("Terminfo: Bad Magic\n");
        // todo: free all
        exit(1);
    }

    // shift value used to calculate offset to end of tables
    // in alloc_info below

    ti_vars->ti_file.wide = (magic == 0x021e) ? 2 : 1;
}

// -----------------------------------------------------------------------

void alloc_info(void)
{
    ti_hdr_t *p;

    int16_t offset;

    map_tifile();           // memory map correct terminfo file
    is_valid();             // verify its magic is correct

    offset = sizeof(ti_hdr_t);

    p = (ti_hdr_t *)ti_vars->ti_file.ti_map;

    // set pointer to names section
    ti_vars->ti_file.ti_names = (char *)&ti_vars->ti_file.ti_map[offset];
    offset += p->ti_names;

    // set pointer to bool section (align if odd length)
    ti_vars->ti_file.ti_bool = &ti_vars->ti_file.ti_map[offset];
    offset += p->ti_bool;
    offset += (offset & 1);

    // set pointer to numbers section which can have 16 or 32 it items
    ti_vars->ti_file.ti_numbers = (int16_t *)&ti_vars->ti_file.ti_map[offset];
    offset += (p->ti_numbers << ti_vars->ti_file.wide);

    // set pointer to strings section which is an array of 16 bit offstts
    // into the table section (below)
    ti_vars->ti_file.ti_strings = (int16_t *)&ti_vars->ti_file.ti_map[offset];
    offset += (p->ti_strings << 1);

    // set address of table section which is a table of escape sequence
    // format strings
    ti_vars->ti_file.ti_table = (char *)&ti_vars->ti_file.ti_map[offset];
}

// =======================================================================
