// uC_screen_resize.c   - resize hold rendering
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_terminfo.h"
#include "uC_alloc.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

extern uC_screen_t *active_screen;
extern uC_attribs_t old_attrs;

void terminfo_purge(void);
void ti_set_screen(uC_screen_t *scr);

// -----------------------------------------------------------------------
// Resize hold support.  On first observed SIGWINCH, snapshot the most
// recently rendered screen, gray it, and keep redrawing that shadow until
// the winch stream is quiet.  The application rebuilds its real UI once
// this function returns true.

#define RESIZE_HOLD_POLL_NS  (25000000)
#define RESIZE_HOLD_QUIET_NS (150000000LL)

// -----------------------------------------------------------------------

static uC_attribs_t resize_shadow_attrs =
{
    .flags.bits = (uC_ATTR_FLAG_GRAY_FG | uC_ATTR_FLAG_GRAY_BG),
    .fg_gray    = uC_GRAY_10,
    .bg_gray    = uC_GRAY_03,
};

// -----------------------------------------------------------------------

static int64_t monotonic_ns(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((int64_t)ts.tv_sec * 1000000000LL) + ts.tv_nsec;
}

// -----------------------------------------------------------------------

static cell_t *resize_snapshot(uC_screen_t *scr)
{
    size_t i;
    size_t count;
    cell_t *shadow;

    if (!scr || !scr->buffer1 || scr->width <= 0 || scr->height <= 0)
    {
        return NULL;
    }

    count = (size_t)scr->width * (size_t)scr->height;
    shadow = uC_alloc(uC_MEM_ZONE_DEFAULT, count * sizeof(*shadow));

    if (!shadow)
    {
        return NULL;
    }

    for (i = 0; i < count; i++)
    {
        shadow[i] = scr->buffer1[i];
        shadow[i].attrs = resize_shadow_attrs;

        if (shadow[i].code == 0)
        {
            shadow[i].code = 0x20;
        }
    }

    return shadow;
}

// -----------------------------------------------------------------------

static void resize_fill_shadow_view(uC_screen_t *view, cell_t *shadow,
    int16_t shadow_w, int16_t shadow_h)
{
    int16_t x;
    int16_t y;
    cell_t blank =
    {
        .attrs = resize_shadow_attrs,
        .code  = 0x20,
    };

    for (y = 0; y < view->height; y++)
    {
        for (x = 0; x < view->width; x++)
        {
            size_t dst = ((size_t)y * (size_t)view->width) + (size_t)x;

            if ((x < shadow_w) && (y < shadow_h))
            {
                size_t src = ((size_t)y * (size_t)shadow_w) + (size_t)x;
                view->buffer1[dst] = shadow[src];
            }
            else
            {
                view->buffer1[dst] = blank;
            }
        }
    }
}

// -----------------------------------------------------------------------

static bool resize_draw_shadow(uC_screen_t *scr, cell_t *shadow)
{
    uint16_t width;
    uint16_t height;
    size_t size;
    uC_screen_t view = { 0 };
    uC_screen_t *restore = active_screen;

    if (!scr || !shadow)
    {
        return false;
    }

    uC_get_console_size(&width, &height);

    if ((width == 0) || (height == 0) ||
        (width > INT16_MAX) || (height > INT16_MAX))
    {
        return false;
    }

    view.width  = (int16_t)width;
    view.height = (int16_t)height;
    view.cx = -1;
    view.cy = -1;

    size = (size_t)view.width * (size_t)view.height * sizeof(cell_t);
    view.buffer1 = uC_alloc(uC_MEM_ZONE_DEFAULT, size);
    view.buffer2 = uC_alloc(uC_MEM_ZONE_DEFAULT, size);

    if (!view.buffer1 || !view.buffer2)
    {
        if (view.buffer1) { uC_free(uC_MEM_ZONE_DEFAULT, view.buffer1); }
        if (view.buffer2) { uC_free(uC_MEM_ZONE_DEFAULT, view.buffer2); }
        return false;
    }

    memset(view.buffer2, 0, size);
    resize_fill_shadow_view(&view, shadow, scr->width, scr->height);

    ti_sgr0();
    old_attrs.blob = 0;
    setlocale(LC_CTYPE, "");

    active_screen = &view;
    ti_set_screen(&view);
    uC_clear();
    scr_normalize_wide_buffer(&view);
    scr_outer_update(&view);

    if (uC_winch_pending())
    {
        terminfo_purge();
    }
    else
    {
        uC_terminfo_flush();
    }

    active_screen = restore;
    ti_set_screen(scr);

    uC_free(uC_MEM_ZONE_DEFAULT, view.buffer1);
    uC_free(uC_MEM_ZONE_DEFAULT, view.buffer2);

    return true;
}

// -----------------------------------------------------------------------

API bool uC_scr_resize_hold(uC_screen_t *scr)
{
    int64_t last_winch;
    cell_t *shadow;

    if (!scr || !uC_winch_pending())
    {
        return false;
    }

#ifdef UC_POPUPS
    uC_scr_popup_cancel(scr);
#endif // UC_POPUPS

    shadow = resize_snapshot(scr);
    uC_winch_ack();
    last_winch = monotonic_ns();

    if (shadow)
    {
        resize_draw_shadow(scr, shadow);
    }

    while (true)
    {
        uC_clock_sleep(RESIZE_HOLD_POLL_NS);

        if (uC_winch_pending())
        {
            uC_winch_ack();
            last_winch = monotonic_ns();

            if (shadow)
            {
                resize_draw_shadow(scr, shadow);
            }
            continue;
        }

        if ((monotonic_ns() - last_winch) >= RESIZE_HOLD_QUIET_NS)
        {
            break;
        }
    }

    if (shadow)
    {
        uC_free(uC_MEM_ZONE_DEFAULT, shadow);
    }

    return true;
}

// =======================================================================
