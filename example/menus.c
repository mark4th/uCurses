// menus.c    - example application menu handling
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>

#include "uCurses.h"
#include "uC_switch.h"
#include "uC_menus.h"
#include "uC_json.h"
#include "uC_utils.h"
#include "uC_keys.h"
#include "uC_status.h"
#include "uC_win_printf.h"

#include "demo.h"

extern uC_screen_t *active_screen;

// -----------------------------------------------------------------------
// just flipflops the border color of which ever window is on top

extern uC_window_t *status_win;

void hello(void);

// -----------------------------------------------------------------------
// user applications can stuff single char keys into the keyboard

static void exit_prog(void)
{
    uC_set_key(0x1b);
}

// -----------------------------------------------------------------------

void window_demo(void)
{
    uC_scr_close(active_screen);
    int status = system("./window");
    hello();
}

// -----------------------------------------------------------------------

void dots_demo(void)
{
    uC_scr_close(active_screen);
    int status = system("./dots");
    hello();
}

// -----------------------------------------------------------------------

void mandel_demo(void)
{
    uC_scr_close(active_screen);
    int status = system("./mandel");
    hello();
}

// -----------------------------------------------------------------------

void lion(void)
{
    uC_scr_close(active_screen);
    int status = system("./lion");
    hello();
}

void raycast(void)
{
    uC_scr_close(active_screen);
    int status = system("./raycast");
    hello();
}

// -----------------------------------------------------------------------

static uC_switch_t menu_vectors[] =
{
    { 0xfcb028fd, window_demo  },
    { 0x9999e2a1, dots_demo    },
    { 0xa3e38b20, mandel_demo  },
    { 0x0920ad33, lion         },
    { 0xaeef1976, raycast      },
    { 0x8d9c616c, exit_prog    }
};

#define VCOUNT sizeof(menu_vectors) / sizeof(menu_vectors[0])

// -----------------------------------------------------------------------
// json parsing calls this to get the address of a specified menu function

opt_t menu_address_cb(int32_t hash)
{
    int16_t i;
    uC_switch_t *s = menu_vectors;

    for(i = 0; i < VCOUNT; i++)
    {
        if(hash == s->option)
        {
            return s->vector;
        }
        s++;
    }

    return NULL;
}

// =======================================================================
