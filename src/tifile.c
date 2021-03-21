// tifle.c   - uCurses terminfo file loading
// -----------------------------------------------------------------------

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

extern uint8_t *esc_buff;

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

uint8_t *ti_map;            // memory mapped address of terminfo file
uint8_t ti_size;            // size of memory mapping

// -----------------------------------------------------------------------

uint8_t *ti_names;          // pointer to term names
uint8_t *ti_bool;           // pointer to terminfo flags
uint8_t *ti_numbers;        // pointer to terminfo numbers
uint16_t *ti_strings;
char *ti_table;

uint8_t wide;               // numbers item size size shift factor

// -----------------------------------------------------------------------

static void map_tifile(void)
{
    int fd;
    const uint8_t *term;
    uint8_t ti_file[128];
    struct stat st;

    term = (uint8_t *)getenv("TERM");
    if(term == NULL)
    {
        printf("No TERM variable set\r\n");
        exit(0);
    }

    strcpy((char *)&ti_file[0], "/usr/share/terminfo/x/");
    ti_file[20] = term[0];
    // magic numbers ftw
    strncat((char *)&ti_file[0], (char *)term, 128-21);

    stat((char *)ti_file, &st);
    ti_size = st.st_size;

    fd = open((char *)ti_file, O_RDONLY, 0);
    if(fd == -1)
    {
        printf("No Terminfo File for %s\r\n %d\n", term, fd);
        exit(1);
    }

    ti_map = (uint8_t *)mmap(NULL, ti_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(ti_map == MAP_FAILED)
    {
        printf("Unable to map Terminfo File\r\n");
        printf(" - %s\r\n", ti_file);
        exit(1);
    }

    close(fd);
}

// -----------------------------------------------------------------------

void init_info()
{
    ti_hdr_t *p;
    uint16_t z;

    uint32_t offset;

    offset = sizeof(ti_hdr_t);

    p = (ti_hdr_t *) &ti_map[0];

    ti_names = &ti_map[offset];
    z = p->ti_names;
    offset += z;

    ti_bool  = &ti_map[offset];
    z = (uint16_t)p->ti_bool;
    offset += z;
    offset += (offset & 1);

    ti_numbers = &ti_map[offset];
    z = (uint16_t)p-> ti_numbers;
    offset += (z << wide);

    ti_strings = (uint16_t *)&ti_map[offset];

    z = (uint16_t)p->ti_strings;
    offset += (z << 1);
    ti_table = (char *)&ti_map[offset];
}

// -----------------------------------------------------------------------

void q_valid(void)
{
    uint16_t magic;

    magic = ((ti_hdr_t *)ti_map)->ti_magic;

    if((magic == 0x011a) || (magic == 0x021e))
    {
        wide = (magic == 0x021e) ? 2 : 1;
    }
}

// -----------------------------------------------------------------------

void uCurses_init(void)
{
    map_tifile();           // memory map correct terminfo file
    q_valid();              // verify its magic is correct

    // allocate 64k for compiled escape sequences
    esc_buff = calloc(1, 65535);

    if(esc_buff == NULL)
    {
        printf("uCurses: insufficient ram for buffers\r\n");
        exit(1);
    }
    init_info();
}

// =======================================================================
