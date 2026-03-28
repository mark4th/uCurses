// uC_json.c  - uCurses json parsing for user interface layout
// -----------------------------------------------------------------------

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>

#include "uCurses.h"
#include "uC_switch.h"
#include "uC_utils.h"
#include "uC_screen.h"
#include "uC_alloc.h"
#include "json.h"

// -----------------------------------------------------------------------

#ifdef UC_JSON

extern uC_screen_t *active_screen;
extern bool winch;

// -----------------------------------------------------------------------
// Custom Json parser state machine

// There are 5 states in this state machine as follows
//
//      JSON_STATE_L_BRACE
//      JSON_STATE_KEY
//      JSON_STATE_VALUE
//      JSON_STATE_R_BRACE
//      JSON_STATE_DONE
//
// As the json text is parsed the text does not define the state, rather,
// the state defines what the next character must be.  All parsing is done
// on a space delimited token by token basis.
//
// The initial state is JSON_STATE_L_BRACE which expects the next token
// parsed to be the left brace '{' char.  If it is then the state is set
// to JSON_STATE_KEY.   If not... oopts!
//
// The handler for JSON_STATE_KEY expect to see one of several known
// tokens which will either be a 'key' token or an 'object' token.  Only
// recogized tokens are valid, unrecognized tokens will cause the state
// machine to puke.
//
// All tokens are recognized by their 32 bit fnv-1a hash value, no string
// compare functions are called.
//
// If the parsed token is recognized as a key...
//
//      That specifc key is handled (see below) and the next state is set
//      to JSON_STATE_VALUE where we extract the value to be stored in the
//      specified key.  note: while the ':' character between a key and
//      its associated value are required there is no state ':'
//
// If the parsed token is one of the recognized objects...
//
//      The state is set once again to JSON_STATE_L_BRACE.
//
// Almost every JSON_STATE_L_BRACE indicates that a new object and thereby
// an associated C structure is being created.
//
// For every object and every key the machine will transition into a new
// state.  If the previous state was an object then that object state is
// pushed onto a state stack.  Key states simply replace the previous key
// state with the new one, they are never pushed onto the state stack.
//
// Any time a new object or key token is parsed we check to see if there
// is a comma on it.  If there is no comma the next state will
// instead be JSON_STATE_R_BRACE which pops the previous state off the
// stack.
//
// As key values are parsed we immediately set the specified item in
// their parents C structure.  When an object is completed we store
// the associated C structure within its parents C structure.  The
// structure types of both specify how and where.
//
// When all states have been popped off the stack we transition into
// JSON_STATE_DONE and the state machine terminates.
//
// The following is WHY I wrote a custom json parser (which literally
// doubled the size of my "micro" sized library)
//
// This code also knows which keys may go inside which objects and which
// objects may go inside which objects.  You cannot for example define
// a screen structure within a window structure.
//
// As stated above, some object states do not have an associated C
// structure.  Any keys specified within these pseudo object will be
// assigned instead to the pseudo objects parent C structure. I.E. the
// keys grandparent.
//
// I have not given a way to define arrays in this parser and I have also
// allowed for values to be expressed as a percentage.  For example you
// can set a window width to a percentage of its parent screens width.

// -----------------------------------------------------------------------
// should i make the state struct a child of the vars struct?

json_vars_t *json_vars;
json_state_t *json_state;

// -----------------------------------------------------------------------
// fnv-1a hash values for various json syntax entities

const int32_t json_syntax[] =
{
    0x050c5d25,             // :
    0x050c5d64,             // {
    0x050c5d62,             // }
};

// -----------------------------------------------------------------------
// push current state onto state stack

static void json_push(json_state_t *j)
{
    uC_list_push_head(&json_vars->json_stack, json_state);
    json_state = j;
}

// -----------------------------------------------------------------------
// de-allocate current state structure and pop previous state off stack.
// the compiled C structure associated with the destroyed state structure
// remains intact

void json_pop(void)
{
    uC_free(uC_MEM_ZONE_JSON, json_state);

    json_state = uC_list_pop_head(&json_vars->json_stack);
}

// -----------------------------------------------------------------------

__attribute__((noreturn)) void json_error(const char *s)
{
    char msg[128];

    sprintf(msg, "%d: %d:%d %s\n",
        json_state->line_no,
        json_vars->line_no,
        json_vars->line_index, s);

    uC_abort(msg);
}

// -----------------------------------------------------------------------
// allocate a new structure.

void *json_alloc(uC_mem_zone_t zone, uint32_t size)
{
    void *v = uC_alloc(zone, size);

    if (v != NULL)
    {
        return v;
    }

    json_error("Out of Memory!");
}

// -----------------------------------------------------------------------
// json files and individual objects must start with a {

static void json_state_l_brace(void)
{
    // assert that the hash value of the most recently parsed json token
    // is equal to the expected left brace hash value

    if (json_vars->json_hash == json_syntax[JSON_L_BRACE])
    {
        json_state->state = JSON_STATE_KEY;
        return;
    }
    json_error("Opening brace missing");
}

// -----------------------------------------------------------------------
// every object and every key gets its own state structure allocation.
// the current object state level is always in the j_state variable.
// previous object states are pushed onto the json parse stack. key value
// states are never pushed onto the state stack

void json_new_state_struct(int struct_size, int32_t struct_type)
{
    void *structure;

    // allocate a structure for the new state
    json_state_t *j = json_alloc(uC_MEM_ZONE_JSON, sizeof(*j));

    // if there is supposed to be one then allocate a buffer for C
    // structure for subsequent keys to populate

    structure = (struct_size != 0)
        ? json_alloc(uC_MEM_ZONE_UI, struct_size)
        : NULL;

    j->parent      = json_state;
    j->structure   = structure;
    j->struct_type = struct_type;
    j->line_no     = json_vars->line_no;

    // push previous state and make new state the current state

    json_push(j);
}

// -----------------------------------------------------------------------
// strips comma and recalculates token hash

static bool check_comma(void)
{
    bool rv = false;

    int16_t end = strlen((char *)json_vars->json_token) - 1;

    if (json_vars->json_token[end] == ',')
    {
        json_vars->json_token[end] = '\0';
        json_vars->json_hash = fnv_hash(json_vars->json_token);
        rv = true;
    }

    return rv;
}

// -----------------------------------------------------------------------

void json_state_r_brace(void)
{
    bool has_comma = check_comma();

    if (json_vars->json_hash != json_syntax[JSON_R_BRACE])
    {
        json_error("Closing brace missing");
    }

    json_state->state = JSON_STATE_DONE;

    if (json_state->struct_type != STRUCT_SCREEN)
    {
        populate_parent();
        json_pop();

        // i am not sure how json_state can ever be null here
        // but I think this was a "bug fix" from way back where
        // i intended to investigate further and forgot

        if (json_state != NULL)
        {
            json_state->state = (has_comma)
                ? JSON_STATE_KEY
                : JSON_STATE_R_BRACE;
        }
    }
}

// -----------------------------------------------------------------------

static const uC_switch_t states[] =
{
    { JSON_STATE_L_BRACE, json_state_l_brace },
    { JSON_STATE_KEY,     json_state_key     },
    { JSON_STATE_VALUE,   json_state_value   },
    { JSON_STATE_R_BRACE, json_state_r_brace },
};

// -----------------------------------------------------------------------

static void run_state_machine(void)
{
    int f;

    do
    {
        if (winch) { return; }

        // read next space delimited token from json data
        token();

        // compute fnv-1a hash for this token

        json_vars->json_hash = fnv_hash(json_vars->json_token);

        // the token does not define what the state is
        // the state defines what the token must be

        f = uC_switch(states, JSON_NUM_STATES, json_state->state);
        if (f == -1)
        {
            json_error("Unknown or out of place JSON token");
        }
    } while (json_state->state != JSON_STATE_DONE);
}

// -----------------------------------------------------------------------

static void json_state_machine(void)
{
    json_state = json_alloc(uC_MEM_ZONE_JSON, sizeof(*json_state));

    json_state->struct_type = -1;
    json_state->state       = JSON_L_BRACE;

    run_state_machine();

    json_pop();
    uC_free(uC_MEM_ZONE_JSON, json_state);
}

// -----------------------------------------------------------------------

static void open_json_file(char *path)
{
    int fd;
    int n;
    struct stat st;

    if (json_vars->json_len != 0)
    {
        return;
    }

    fd = stat(path, &st);

    if (fd != -1)
    {
        fd = open(path, O_RDONLY);

        if (fd != -1)
        {
            json_vars->json_len  = st.st_size;
            json_vars->json_data = uC_alloc(uC_MEM_ZONE_DEFAULT,
                st.st_size);
            uC_ASSERT(json_vars->json_data != NULL, "Out Of Memory!");

            n = read(fd, json_vars->json_data, st.st_size);
            close(fd);

            if (n == st.st_size)
            {
                return;
            }
        }

        uC_free(uC_MEM_ZONE_DEFAULT, json_vars->json_data);
    }
    json_vars->json_len  = 0;
    json_vars->json_data = 0;
    json_error("Unable to map JSON file");
}

// -----------------------------------------------------------------------

static void parse_json_data(void)
{
    json_state_machine();

    if (active_screen != NULL)
    {
        json_build_ui();
    }
}

// -----------------------------------------------------------------------

// when the state machine parses in the structues that define the app
// menus it will need to add function pointers to every menu item to be
// executed when that menu item is selected.  this library can not
// determine the address of any functions in the application code it is
// linked against.   said application will need to supply this library
// with a pointer to a function that will return a pointer to a menu
// function based off of a string.  The HOW of this needs better
// documentation than im prepared to put in source file comments :)

void json_file_create_ui(char *path, fp_finder_t fp)
{
    json_vars = uC_alloc(uC_MEM_ZONE_JSON, sizeof(*json_vars));

    json_vars->json_stack.zone = uC_MEM_ZONE_UI;

    uC_get_console_size(&json_vars->console_width,
        &json_vars->console_height);

    json_vars->fp_finder = fp;

    open_json_file(path);

    json_de_tab(json_vars->json_data, json_vars->json_len);
    json_vars->line_no = 1;

    parse_json_data();

    uC_free(uC_MEM_ZONE_DEFAULT, json_vars->json_data);
    json_vars->json_data = 0;

    uC_free(uC_MEM_ZONE_JSON, json_vars);
}

// -----------------------------------------------------------------------
// allows users to embed their json data into their .data section etc

void json_mem_create_ui(char *json_data, int len, fp_finder_t fp)
{
    json_vars = uC_alloc(uC_MEM_ZONE_JSON, sizeof(*json_vars));

    uC_get_console_size(&json_vars->console_width,
        &json_vars->console_height);

    json_vars->json_data       = json_data;
    json_vars->json_len        = len;
    json_vars->fp_finder       = fp;
    json_vars->json_stack.zone = uC_MEM_ZONE_UI;
    json_vars->json_stack.zone = uC_MEM_ZONE_UI;

    parse_json_data();

    uC_free(uC_MEM_ZONE_JSON, json_vars);
}

// -----------------------------------------------------------------------

#endif

// =======================================================================
