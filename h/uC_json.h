// json.h
// -----------------------------------------------------------------------

#ifndef UC_JSON_H
#define UC_JSON_H

#include <stdbool.h>

#include "uC_menus.h"
#include "uC_list.h"

// -----------------------------------------------------------------------

#define MAX_LINE_LEN  (128)
#define MAX_TOKEN_LEN (35) // 32 chars plus two quote chars and a null

// -----------------------------------------------------------------------

enum
{
    WIN_FAR  = 0x7fff
};

// -----------------------------------------------------------------------

typedef enum
{
    STATE_NONE,
    STATE_L_BRACE,
    STATE_KEY,
    STATE_VALUE,
    STATE_R_BRACE,
    STATE_DONE,
    NUM_STATES
} json_states;

// -----------------------------------------------------------------------
// should really be two enums but that just complificates the code

typedef enum
{
    STRUCT_SCREEN = 1,
    STRUCT_WINDOW,
    STRUCT_BACKDROP,
    STRUCT_MENU_BAR,
    STRUCT_PULLDOWN,
    STRUCT_MENU_ITEM,
    STRUCT_ATTRIBS,         // normal attribs
    STRUCT_B_ATTRIBS,       // border attribs
    STRUCT_S_ATTRIBS,       // selected attribs
    STRUCT_D_ATTRIBS,       // disabled attribs
    STRUCT_FLAGS,
    STRUCT_RGB_FG,          // 3 bytes
    STRUCT_RGB_BG,          // 3 bytes
    STRUCT_WINDOWS,
    STRUCT_PULLDOWNS,
    STRUCT_MENU_ITEMS,

    KEY_FG,    KEY_BG,          KEY_GRAY_BG,  KEY_GRAY_FG,
    KEY_RED,   KEY_GREEN,       KEY_BLUE,     KEY_XCO,
    KEY_YCO,   KEY_WIDTH,       KEY_HEIGHT,   KEY_NAME,
    KEY_FLAGS, KEY_BORDER_TYPE, KEY_VECTOR,   KEY_SHORTCUT,
    KEY_FLAG
} json_type_t;

// -----------------------------------------------------------------------
// offsets into json_syntax array of hash values found in json.c

typedef enum
{
    JSON_COLON,
    JSON_L_BRACE,
    JSON_R_BRACE,
    JSON_L_BRACKET,
    JSON_R_BRACKET
} json_syntax_t;

// -----------------------------------------------------------------------

typedef struct
{
    char *json_data;        // pointer to json data to be parsed
    int json_len;           // total size of json data
    int json_index;         // parse index into data (current line)
    int32_t key_value;
    bool percent;           // key value is expressed as a percentage
    bool quoted;

    char line_buff[MAX_LINE_LEN];

    int16_t line_no;        // current line of json data
    int16_t line_index;     // line parse location
    int16_t line_left;      // number of chars left to parse in line

    // space delimited token extracted from data - +1 for the null

    char json_token[MAX_TOKEN_LEN];
    int32_t json_hash;

    uC_list_t json_stack;

    fp_finder_t fp_finder;

    int16_t console_width;
    int16_t console_height;
} json_vars_t;

// -----------------------------------------------------------------------

typedef struct
{
    int state;                // current state
    void *parent;             // pointer to parent j_state_t
    void *structure;          // pointer to structure being populated
    json_type_t struct_type;  // type of structure being populated
    int16_t line_no;          // line number where struct was initiated
} json_state_t;

// -----------------------------------------------------------------------
// visibility hidden

__attribute__((noreturn)) void json_error(const char *s);

void json_pop(void);
void *json_alloc(uint32_t size);
void json_new_state_struct(int struct_size, int32_t struct_type);
void json_state_r_brace(void);
void json_build_ui(void);
void json_state_key(void);
void populate_parent(void);
void strip_quotes(int16_t len);
void json_de_tab(char *s, int len);
void token(void);
void json_state_value(void);

// -----------------------------------------------------------------------

API void uC_json_create_ui(char *path, fp_finder_t fp);

// -----------------------------------------------------------------------

#endif // UC_JSON_H

// =======================================================================
