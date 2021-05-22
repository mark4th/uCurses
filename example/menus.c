// menus.c    - example application menu handling
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdlib.h>

#include "../h/uCurses.h"

#include "demo.h"

// -----------------------------------------------------------------------
// just flipflops the border color of which ever window is on top

static void window_demo(void)
{
    node_t *n;
    window_t *win1;
    window_t *win2;
    screen_t *scr;

    scr_close(active_screen);
    json_create_ui("demo1.json", menu_address_cb);
    scr = active_screen;

    n = scr->windows.head;
    win1 = n->payload;
    n = n->next;
    win2 = n->payload;

    alloc_status();
    run_demo1(scr, win1, win2);
    scr_close(active_screen);
    main_screen();
}

// -----------------------------------------------------------------------

static void dots_demo(void)
{
    scr_close(active_screen);
    json_create_ui("dots.json", menu_address_cb);

    alloc_status();
    do_dots();
    scr_close(active_screen);
    main_screen();
}

// -----------------------------------------------------------------------
// user applications can stuff single char keys into the keyboard

static void exit_prog(void)
{
    stuff_key(0x1b);
}

// -----------------------------------------------------------------------

static switch_t menu_vectors[] =
{
    { 0xfcb028fd, window_demo             },
    { 0x9999e2a1, dots_demo               },
    { 0xa3e38b20, mandel_demo             },
    { 0x0920ad33, lion                    },
    { 0xaeef1976, raycast                 },
    { 0x8d9c616c, exit_prog               }
};

#define VCOUNT sizeof(menu_vectors) / sizeof(menu_vectors[0])

// -----------------------------------------------------------------------
// json parsing calls this to get the address of a specified menu function

opt_t menu_address_cb(int32_t hash)
{
    int16_t i;
    switch_t *s = menu_vectors;

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
