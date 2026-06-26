// uC_shortcuts.c
// -----------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

#include "uC_alloc.h"
#include "uC_keys.h"
#include "uC_list.h"
#include "uC_screen.h"
#include "uC_utils.h"
#ifdef UC_WIDGETS
#include "uC_widgets.h"
#endif

// -----------------------------------------------------------------------

typedef struct
{
    uC_shortcut_t shortcut;
    uC_shortcut_t normalized;
    uC_shortcut_action_t *action;
    void *context;
    void *owner;
} uC_shortcut_entry_t;

// -----------------------------------------------------------------------

static uint8_t shortcut_key(uC_shortcut_t shortcut)
{
    return (uint8_t)(shortcut & UC_SHORTCUT_KEY_MASK);
}

// -----------------------------------------------------------------------

static bool shortcut_alpha(uint8_t key)
{
    return ((key >= 'A') && (key <= 'Z')) ||
           ((key >= 'a') && (key <= 'z'));
}

// -----------------------------------------------------------------------

static uint8_t shortcut_upper(uint8_t key)
{
    if ((key >= 'a') && (key <= 'z'))
    {
        return (uint8_t)(key - ('a' - 'A'));
    }
    return key;
}

// -----------------------------------------------------------------------

static uC_shortcut_t shortcut_normalize(uC_shortcut_t shortcut)
{
    uint8_t key = shortcut_key(shortcut);
    uC_shortcut_t mods = shortcut & ~UC_SHORTCUT_KEY_MASK;

    return mods | shortcut_upper(key);
}

// -----------------------------------------------------------------------

static bool shortcut_plain_match(uint8_t shortcut, uint8_t key)
{
    if (shortcut == key)
    {
        return true;
    }

    if (shortcut_alpha(shortcut) && shortcut_alpha(key))
    {
        return shortcut_upper(shortcut) == shortcut_upper(key);
    }

    return false;
}

// -----------------------------------------------------------------------

static bool shortcut_ctrl_code(uint8_t key, uint8_t *out)
{
    key = shortcut_upper(key);

    if ((key >= '@') && (key <= '_'))
    {
        *out = (uint8_t)(key & 0x1f);
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------

bool uC_shortcut_matches(uC_shortcut_t shortcut, uint8_t key)
{
    uint8_t shortcut_base = shortcut_key(shortcut);
    uint8_t ctrl_key;

    if ((shortcut == 0) || (shortcut_base == 0) || (key == 0))
    {
        return false;
    }

    if ((shortcut & (UC_SHORTCUT_MOD_ALT | UC_SHORTCUT_MOD_META)) != 0)
    {
        return false;
    }

    if ((shortcut & UC_SHORTCUT_MOD_CTRL) != 0)
    {
        if (!shortcut_ctrl_code(shortcut_base, &ctrl_key))
        {
            return false;
        }
        return key == ctrl_key;
    }

    return shortcut_plain_match(shortcut_base, key);
}

// -----------------------------------------------------------------------

bool uC_shortcut_register(uC_screen_t *scr,
    uC_shortcut_t shortcut, uC_shortcut_action_t *action, void *context,
    void *owner)
{
    uC_list_node_t *node;
    uC_shortcut_entry_t *entry;
    uC_shortcut_t normalized;

    if ((scr == NULL) || (shortcut == 0) || (action == NULL))
    {
        return false;
    }

    normalized = shortcut_normalize(shortcut);
    for (node = uC_list_scan(&scr->shortcuts, NULL);
         node != NULL;
         node = uC_list_scan(NULL, node))
    {
        entry = node->payload;
        if (entry && entry->normalized == normalized)
        {
            return false;
        }
    }

    entry = uC_alloc(uC_MEM_ZONE_UI, sizeof(*entry));
    if (entry == NULL)
    {
        return false;
    }

    entry->shortcut = shortcut;
    entry->normalized = normalized;
    entry->action = action;
    entry->context = context;
    entry->owner = owner;

    if (!uC_list_push_tail(&scr->shortcuts, entry))
    {
        uC_ui_free(entry);
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------

bool uC_shortcut_run(uC_screen_t *scr, uint8_t key)
{
    uC_list_node_t *node;
    uC_shortcut_entry_t *entry;

    if (scr == NULL)
    {
        return false;
    }

    for (node = uC_list_scan(&scr->shortcuts, NULL);
         node != NULL;
         node = uC_list_scan(NULL, node))
    {
        entry = node->payload;
        if (entry && uC_shortcut_matches(entry->shortcut, key))
        {
            uC_set_key(0xff);
            entry->action(entry->context);
            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------

bool uC_shortcut_run_popup(uC_screen_t *scr, uint8_t key)
{
#ifdef UC_WIDGETS
    uC_list_node_t *node;
    uC_shortcut_entry_t *entry;
    uC_widget_vg_t *popup;

    if ((scr == NULL) || (scr->popup_vg == NULL))
    {
        return false;
    }

    popup = (uC_widget_vg_t *)scr->popup_vg;
    for (node = uC_list_scan(&scr->shortcuts, NULL);
         node != NULL;
         node = uC_list_scan(NULL, node))
    {
        entry = node->payload;
        if (entry && uC_shortcut_matches(entry->shortcut, key) &&
            widget_vg_contains_widget(popup, (uC_widget_t *)entry->owner))
        {
            uC_set_key(0xff);
            entry->action(entry->context);
            return true;
        }
    }
#else
    (void)scr;
    (void)key;
#endif

    return false;
}

// -----------------------------------------------------------------------

void uC_shortcut_remove_owner(uC_screen_t *scr, void *owner)
{
    uC_list_node_t *node;
    uC_list_node_t *next;
    uC_shortcut_entry_t *entry;

    if ((scr == NULL) || (owner == NULL))
    {
        return;
    }

    node = uC_list_scan(&scr->shortcuts, NULL);
    while (node != NULL)
    {
        next = uC_list_scan(NULL, node);
        entry = node->payload;
        if (entry && entry->owner == owner)
        {
            uC_list_remove_node(&scr->shortcuts, entry);
            uC_ui_free(entry);
        }
        node = next;
    }
}

// -----------------------------------------------------------------------

void uC_shortcut_clear(uC_screen_t *scr)
{
    uC_shortcut_entry_t *entry;

    if (scr == NULL)
    {
        return;
    }

    while (scr->shortcuts.count != 0)
    {
        entry = uC_list_pop_head(&scr->shortcuts);
        uC_ui_free(entry);
    }
}

// =======================================================================
