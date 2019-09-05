// tifle.c   - uCurses terminfo file loading
// -----------------------------------------------------------------------

#include <sys/mman.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "h/uCurses.h"
#include "h/uCurses_types.h"

// -----------------------------------------------------------------------

extern uint8_t *buffer;
extern fp_t send_str;

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
uint8_t *ti_table;

uint8_t wide;               // numbers item size size shift factor

// -----------------------------------------------------------------------

static void map_tifile(void)
{
  uint32_t fd;
  const uint8_t *term;
  uint8_t ti_file[128];
  struct stat st;

  term = (cu8_ptr)getenv("TERM");
  if(NULL == term)
  {
    printf("No TERM variable set\r\n");
    exit(0);
  }

  strcpy((str)&ti_file[0], "/usr/share/terminfo/x/");
  ti_file[20] = term[0];
  strncat((str)&ti_file[0], (cstr)term, 128-21);  // magic numbers ftw

  stat((cstr)ti_file, &st);
  ti_size = st.st_size;

  fd = open((cstr)ti_file, O_RDONLY, 0);
  if(-1 == fd)
  {
    printf("No Terminfo File for %s\r\n", term);
    exit(1);
  }

  ti_map = (u8_ptr)mmap(NULL, ti_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if(MAP_FAILED == ti_map)
  {
    printf("Unable to map Terminfo File\r\n");
    printf(" - %s\r\n", ti_file);
    exit(1);
  }

  printf("Successfully maped Terminfo FIle\r\n");

  printf(" - %s\r\n", ti_file);

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
  ti_table = &ti_map[offset];
}

// -----------------------------------------------------------------------

void q_valid(void)
{
  uint16_t magic;

  magic = ((ti_hdr_t *)ti_map)->ti_magic;

  if((0x011a == magic) || (0x021e == magic))
  {
    wide = (0x021e == magic) ? 2 : 1;
  }
}

// -----------------------------------------------------------------------

void uCurses_init(void)
{
  map_tifile();             // memory map correct terminfo file
  q_valid();                // verify its magic is correct

  send_stdout = &_send_stdout;          // initialize function pointers

  buffer = (u8_ptr)malloc(65535);   // allocate 64k for compiled escape sequences

  if(NULL == buffer)        // if we can!
  {
    printf("uCurses: insufficient ram for buffers\r\n");
    exit(0);
  }
  init_info();
  ti_enacs();               // for those occasions where its required
}

// =======================================================================
