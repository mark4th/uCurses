// uC_widget_button.c
// -----------------------------------------------------------------------

#include "uCurses.h"
#include "uC_screen.h"
#include "uC_widgets.h"

// -----------------------------------------------------------------------

#ifdef UC_WIDGETS

// -----------------------------------------------------------------------

extern widget_state_t widget_state;

// -----------------------------------------------------------------------
// a key has been pressed while a button has focus

uint8_t handle_button(uint8_t k)
{
    uC_widget_button_t *b;

    if (k == 0x0a)
    {
        b = &widget_state.widget->button;
        k = b->letter;
    }

    return k;
}

// -----------------------------------------------------------------------

API uC_widget_t *uC_widget_button_create(
    uint16_t sequence, uint16_t *select,
    char *name, char letter,
    uint16_t width, uint8_t xco, uint8_t yco,
    uC_attribs_t attrs, uC_attribs_t focus)
{
    uC_widget_t *w = create_widget(uC_WIDGET_BUTTON, name,
        sequence, xco, yco, width, attrs, focus);

    if (w != NULL)
    {
        // text displayed on button and optional keybord shortcut

        w->button.letter = letter;
        w->button.select = select;
    }

    return w;
}

// -----------------------------------------------------------------------

#endif // UC_WIDGETS

// =======================================================================
