// uC_json_populate.c   - populate parent structures with child structure
// -----------------------------------------------------------------------

#include <inttypes.h>

#include "uCurses.h"
#include "uC_window.h"
#include "uC_screen.h"
#include "uC_utils.h"
#include "json.h"

#ifdef UC_JSON

// -----------------------------------------------------------------------

extern json_state_t *json_state;

// -----------------------------------------------------------------------
// copy completed child attribs into the nominated parent field, then free

static void copy_attribs_to(uC_attribs_t *dest)
{
    *dest = *(uC_attribs_t *)json_state->structure;
    uC_ui_free(json_state->structure);
}

// -----------------------------------------------------------------------
// an object has been completed and its C structure is ready.
// wire it into its parent (or grandparent) structure.

void populate_parent(void)
{
    json_state_t *parent = json_state->parent;
    json_state_t *gp     = parent->parent;
    void         *ps     = parent->structure;   // parent struct
    void         *gs     = gp->structure;       // grandparent struct
    json_type_t   pt     = parent->struct_type;

    switch (json_state->struct_type)
    {
        case STRUCT_ATTRIBS:
        {
            uC_attribs_t *dest =
                (pt == STRUCT_PULLDOWN) ? &((pulldown_t  *)ps)->attrs :
                (pt == STRUCT_MENU_BAR) ? &((menu_bar_t  *)ps)->attrs :
                                          &((uC_window_t *)ps)->attrs;
            copy_attribs_to(dest);
            break;
        }
        case STRUCT_B_ATTRIBS:
            copy_attribs_to(&((uC_window_t *)ps)->bdr_attrs);
            break;
        case STRUCT_F_ATTRIBS:
            copy_attribs_to(&((uC_window_t *)ps)->focus_attrs);
            break;
        case STRUCT_S_ATTRIBS:
        {
            uC_attribs_t *dest =
                (pt == STRUCT_PULLDOWN) ? &((pulldown_t *)ps)->selected_attrs :
                                          &((menu_bar_t *)ps)->selected_attrs;
            copy_attribs_to(dest);
            break;
        }
        case STRUCT_D_ATTRIBS:
        {
            uC_attribs_t *dest =
                (pt == STRUCT_PULLDOWN) ? &((pulldown_t *)ps)->disabled_attrs :
                                          &((menu_bar_t *)ps)->disabled_attrs;
            copy_attribs_to(dest);
            break;
        }
        case STRUCT_PULLDOWN:
        {
            menu_bar_t *bar = gs;
            bar->items[bar->count++] = json_state->structure;
            break;
        }
        case STRUCT_MENU_ITEM:
        {
            pulldown_t *pd = gs;
            pd->items[pd->count++] = json_state->structure;
            break;
        }
        case STRUCT_WINDOW:
            uC_scr_win_attach(gs, json_state->structure);
            break;
        case STRUCT_BACKDROP:
            ((uC_screen_t *)ps)->backdrop = json_state->structure;
            break;
        case STRUCT_MENU_BAR:
            ((uC_screen_t *)ps)->menu_bar = json_state->structure;
            break;
        default:
            break;
    }
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
