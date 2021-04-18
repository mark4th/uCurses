// uCurses json parsing for user interface layout
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "h/uCurses.h"
#include "h/list.h"

// -----------------------------------------------------------------------

char *json_data;        // pointer to json data to be parsed
size_t json_len;        // total size of json data
uint32_t json_index;    // parse index into data (current line)

char line_buff[MAX_LINE_LEN];
uint16_t line_no;
uint16_t line_index;    // line parse location
uint16_t line_left;     // number of chars left to parse in line

char json_token[TOKEN_LEN]; // space delimited token extracted from data
uint32_t json_hash;

list_t j_stack;

fp_finder_t fp_finder;

// -----------------------------------------------------------------------

j_state_t *j_state;

// -----------------------------------------------------------------------
// json syntax

const uint32_t json_syntax[] =
{
//    0x050c5d3c,             // #
    0x050c5d25,             // :
    0x050c5d64,             // {
    0x050c5d62,             // }
    0x050c5d44,             // [
    0x050c5d42              // ]
};

// -----------------------------------------------------------------------

static void j_push(j_state_t *j)
{
    list_append_node(&j_stack, j_state);
    j_state = j;
}

// -----------------------------------------------------------------------

void j_pop(void)
{
    free(j_state);

    j_state = list_pop(&j_stack);
}

// -----------------------------------------------------------------------

void json_error(char *s)
{
    // printf offending line and pointer to offending item?
    fprintf(stderr, "%d:%d %s\n", line_no, line_index, s);
    _exit(1);
}

// -----------------------------------------------------------------------

void *j_alloc(uint32_t size)
{
    void *v = calloc(1, size);

    if(v == NULL)
    {
        json_error("Out of Memory!");
    }
    return v;
}

// -----------------------------------------------------------------------
// json files and individual objects must start with a {

static void json_state_l_brace(void)
{
    if(json_hash != json_syntax[JSON_L_BRACE])
    {
        json_error("Opening brace missing");
    }

    j_state->state++;
}

// -----------------------------------------------------------------------
// every object and every key gets its own state structure
// the current object state level is alwaus in j_state.  previous
// states are pushed onto the json parse stack

void json_new_state_struct(size_t struct_size, uint32_t struct_type)
{
    j_state_t *j;

    void *structure = NULL;

    j = j_alloc(sizeof(*j));

    if(struct_size != 0)
    {
        structure = j_alloc(struct_size);
    }

    j->parent      = j_state;
    j->structure   = structure;
    j->struct_type = struct_type;

    j_push(j);
}

// -----------------------------------------------------------------------
// classic example of why i hate c switch statements :P

static void populate_parent(void)
{
    uint16_t i;
    list_t *l;
    j_state_t *parent = j_state->parent;
    void *structure = parent->structure;
    uint32_t ptype    = parent->struct_type;

    switch(j_state->struct_type)
    {
        case STRUCT_ATTRIBS:
            switch(ptype)
            {
                case STRUCT_WINDOW:
                case STRUCT_BACKDROP:
                    *(uint64_t *)((window_t *)structure)->attrs = *(uint64_t *)j_state->structure;
                    break;
                case STRUCT_PULLDOWN:
                    *(uint64_t *)((pulldown_t *)structure)->normal = *(uint64_t *)j_state->structure;
                    break;
                case STRUCT_MENU_BAR:
                    *(uint64_t *)((menu_bar_t *)structure)->normal = *(uint64_t *)j_state->structure;
                    break;
            }
            break;
        case STRUCT_B_ATTRIBS:
               *(uint64_t *)((window_t *)structure)->bdr_attrs = *(uint64_t *)j_state->structure;
                break;
        case STRUCT_S_ATTRIBS:
            switch(ptype)
            {
                case STRUCT_PULLDOWN:
                    *(uint64_t *)((pulldown_t *)structure)->selected = *(uint64_t *)j_state->structure;
                    break;
                case STRUCT_MENU_BAR:
                    *(uint64_t *)((menu_bar_t *)structure)->selected = *(uint64_t *)j_state->structure;
                    break;
            }
            break;
        case STRUCT_D_ATTRIBS:
            switch(ptype)
            {
                case STRUCT_PULLDOWN:
                    *(uint64_t *)((pulldown_t *)structure)->disabled = *(uint64_t *)j_state->structure;
                    break;
                case STRUCT_MENU_BAR:
                    *(uint64_t *)((menu_bar_t *)structure)->disabled = *(uint64_t *)j_state->structure;
                    break;
            }
            break;
        case STRUCT_PULLDOWN:
            i = ((menu_bar_t *)structure)->count++;
            ((menu_bar_t *)structure)->items[i] = j_state->structure;
            break;
        case STRUCT_MENU_ITEM:
            i = ((pulldown_t *)structure)->count++;
            ((pulldown_t *)structure)->items[i] = j_state->structure;
            break;
        case STRUCT_WINDOW:
            l = &((screen_t *)structure)->windows;
            list_append_node(l, j_state->structure);
            break;
    }
}

// -----------------------------------------------------------------------

static void json_state_r_brace(void)
{
    uint16_t has_comma = 0;
    uint16_t end = strlen(json_token) - 1;

    if(json_token[end] == ',')
    {
        json_token[end] = '\0';
        json_hash = fnv_hash(json_token);
        has_comma = 1;
    }

    if(json_hash != json_syntax[JSON_R_BRACE])
    {
        json_error("Closing brace missing");
    }

    populate_parent();

    if(j_stack.count != 0)
    {
        j_pop();

        j_state->state = (has_comma != 0)
            ? STATE_KEY
            : STATE_R_BRACE;
     }
    else
    {
        // if has comma json_error("id10t") ?
        j_state->state = STATE_DONE;
    }
}

// -----------------------------------------------------------------------

static const switch_t states[] =
{
    { STATE_L_BRACE,  json_state_l_brace },
    { STATE_KEY,      json_state_key     },
    { STATE_VALUE,    json_state_value   },
    { STATE_R_BRACE,  json_state_r_brace },
};

// -----------------------------------------------------------------------

void json_state_machine(char *json, size_t len, fp_finder_t fp)
{
    fp_finder = fp;
    j_state = j_alloc(sizeof(*j_state));

    json_data = json;
    json_len  = len;

    j_state->struct_type = -1;
    j_state->state = JSON_L_BRACE;

    json_de_tab(json, len);

    while(j_state->state != STATE_DONE)
    {
        token();
        json_hash = fnv_hash(json_token);

        re_switch(states, NUM_STATES, j_state->state);
    }
    // TODO
    // allocate screen and window structures backing stores here
    // because we know there sizes now
}

// =======================================================================
