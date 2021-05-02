// tifle.c   - uCurses terminfo file loading
// -----------------------------------------------------------------------

#include <fcntl.h>
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

extern FILE *log_fp;

extern uint8_t *esc_buff;
struct termios term_save;
struct termios term;

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

char *paths[] =
{
    "/usr/share/terminfo/", // the RIGHT place for terminfo files
    "/lib/terminfo/",       // the stupidest place to put it
    "/etc/terminfo/",       // better than /lib debian idiocy
};

// -----------------------------------------------------------------------

static void map_tifile(void)
{
    uint16_t i;
    uint16_t len;
    int fd;
    const char *env_term;
    char path[128];
    struct stat st;

    env_term = getenv("TERM");
    if(env_term == NULL)
    {
        printf("No TERM variable set\r\n");
        exit(0);
    }

    for(i = 0; i < 3; i++)
    {
        memset(path, 0, 128);
        len = strlen(paths[i]);
        strncpy(path, paths[i], len);
        path[len++] = env_term[0];
        path[len]   = '/';
        len = strlen(env_term);
        strncat(path, env_term, len);

        stat(path, &st);
        ti_size = st.st_size;

        fd = open((char *)path, O_RDONLY, 0);
        if(fd == -1) { continue; }

        ti_map = (uint8_t *)mmap(NULL, ti_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if(ti_map == MAP_FAILED)
        {
            printf("Unable to map Terminfo File\r\n");
            printf(" - %s\r\n", path);
            exit(1);
        }
        close(fd);

        // log_fp = fopen("log.bin", "w");
        return;
    }

    printf("No Terminfo File for %s\n", env_term);
    exit(1);
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

    setlocale(LC_ALL, "C.UTF-8");
    curoff();
    tcgetattr(STDIN_FILENO, &term_save);
    term = term_save;
    term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// -----------------------------------------------------------------------

void uCurses_deInit(void)
{
    free(esc_buff);

    munmap(ti_map, ti_size);
}

// =======================================================================
