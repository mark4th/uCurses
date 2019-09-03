// tui.h
// -----------------------------------------------------------------------

#include <inttypes.h>

// -----------------------------------------------------------------------

#define CELL sizeof(uint32_t)

// -----------------------------------------------------------------------

typedef struct
{
  void *windows;            // linked list of windows
  uint32_t *buffer1;         // screen buffer 1 and 2
  uint32_t *buffer2;
  uint16_t width;           // screen dimensions
  uint16_t height;
  uint16_t flags;           // not defined yet
  uint16_t reserved;
} screen_t;

// -----------------------------------------------------------------------

typedef enum
{
  boxed,
  locked,
  filled
} win_flag_t;

// -----------------------------------------------------------------------

typedef struct
{
  screen_t *screen;
  uint32_t *buffer;
  win_flag_t flags;
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  uint16_t cx;
  uint16_t cy;
  uint8_t attr;
  uint8_t colors;
  uint8_t battr;
  uint8_t bcolors;
  uint8_t blank;
  uint8_t reserved[3];
} window_t;

// -----------------------------------------------------------------------

void win_attach(screen_t *s, window_t *w);
void win_detach(window_t *w);

// =======================================================================
