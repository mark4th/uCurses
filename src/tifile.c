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

extern int8_t *esc_buff;
extern uint16_t num_esc;

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

int8_t *ti_map;  // memory mapped address of terminfo file
int16_t ti_size; // size of memory mapping

// -----------------------------------------------------------------------

char *ti_names;      // pointer to term names
int8_t *ti_bool;     // pointer to terminfo flags
int16_t *ti_numbers; // pointer to terminfo numbers
int16_t *ti_strings;
char *ti_table;

int8_t wide; // numbers item size size shift factor

// -----------------------------------------------------------------------
// terminfo can also be in ~/terminfo but who puts it there?

char *paths[] = //
    {
        "/usr/share/terminfo/", // the RIGHT place for terminfo files
        "/lib/terminfo/",       // the stupidest place to put it
        "/etc/terminfo/",       // better than /lib debian idiocy
    };

// -----------------------------------------------------------------------

static void map_tifile(void)
{
    int16_t i;
    int16_t len;
    int fd;
    const char *env_term;
    char path[128];
    struct stat st;

    // only if you want HUGE logs of every single character
    // written to the console :)

    // log_fp = fopen("log", "w");

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
        path[len] = '/';
        len = strlen(env_term);
        strncat(path, env_term, len);

        stat(path, &st);
        ti_size = st.st_size;

        fd = open((char *)path, O_RDONLY, 0);
        if(fd == -1)
        {
            continue;
        }

        ti_map =
            (int8_t *)mmap(NULL, ti_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);

        if(ti_map != MAP_FAILED)
        {
            return;
        }
    }

    printf("No Terminfo File found for %s\n", env_term);
    exit(1);
}

// -----------------------------------------------------------------------

void init_info()
{
    ti_hdr_t *p;

    int16_t offset;

    offset = sizeof(ti_hdr_t);

    p = (ti_hdr_t *)ti_map;

    // set pointer to names section
    ti_names = (char *)&ti_map[offset];
    offset += p->ti_names;

    // set pointer to bool section (align if odd length)
    ti_bool = &ti_map[offset];
    offset += p->ti_bool;
    offset += (offset & 1);

    // set pointer to numbers section which can have 16 or 32 it items
    ti_numbers = (int16_t *)&ti_map[offset];
    offset += (p->ti_numbers << wide);

    // set pointer to strings section which is an array of 16 bit offstts
    // into the table section (below)
    ti_strings = (int16_t *)&ti_map[offset];
    offset += (p->ti_strings << 1);

    // set address of table section which is a table of escape sequence
    // format strings
    ti_table = (char *)&ti_map[offset];
}

// -----------------------------------------------------------------------

void q_valid(void)
{
    int16_t magic;

    magic = ((ti_hdr_t *)ti_map)->ti_magic;

    if((magic == 0x011a) || (magic == 0x021e))
    {
        // shift value used to calculate offset to end of tables
        // in init_info() above
        wide = (magic == 0x021e) ? 2 : 1;
    }
}

// -----------------------------------------------------------------------

void uCurses_init(void)
{
    map_tifile(); // memory map correct terminfo file
    q_valid();    // verify its magic is correct

    // allocate 64k for compiled escape sequences
    esc_buff = calloc(1, 65535);

    if(esc_buff == NULL)
    {
        printf("uCurses: insufficient ram for buffers\r\n");
        exit(1);
    }
    init_info();

    num_esc = 0;

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
