// menus.c    - example application menu handling
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include "uCurses.h"
#include "uC_switch.h"
#include "uC_menus.h"
#include "uC_utils.h"
#include "uC_keys.h"
#include "uC_status.h"
#include "uC_win_printf.h"

#include "demo.h"

extern uC_screen_t *active_screen;
extern uC_window_t *status_win;

// -----------------------------------------------------------------------

void hello(void);
void my_winch(void);

// -----------------------------------------------------------------------
// user applications can stuff single char keys into the keyboard

static void exit_prog(void)
{
    uC_set_key(0x1b);
}

// -----------------------------------------------------------------------

static void run_demo(char *demo)
{
//    uC_deregister_winch(my_winch);
    uC_restore_terminal();

    int s = system(demo);

    uC_init_terminal();
    uC_curoff();
    uC_flush_keys();
    uC_clear();
//    uC_register_winch(my_winch);
}

// -----------------------------------------------------------------------

void window_demo(void) { run_demo("./window");  }
void dots_demo(void)   { run_demo("./dots");    }
void mandel_demo(void) { run_demo("./mandel");  }
void lion(void)        { run_demo("./lion");    }
void raycast(void)     { run_demo("./raycast"); }

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
