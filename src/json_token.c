// json parsing - token extraction and hash calculation
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include "h/uCurses.h"

// -----------------------------------------------------------------------

#define FNV_PRIME 0x01000193
#define FNV_BASIS 0x811c9dc5

extern j_state_t *j_state;

extern char *json_data;        // pointer to json data to be parsed
extern size_t json_len;        // total size of json data
extern uint32_t json_index;    // parse index into data (current line)
extern char line_buff[MAX_LINE_LEN];
extern uint16_t line_no;
extern uint16_t line_index;    // line parse location
extern uint16_t line_left;     // number of chars left to parse in line
extern char json_token[TOKEN_LEN];    // space delimited token extracted from data
extern uint32_t json_hash;

// -----------------------------------------------------------------------
// FNV-1a on utf8 strings

uint32_t fnv_hash(char *s)
{
    uint32_t hash = FNV_BASIS;

    uint8_t len;

    while(*s != '\0')
    {
        len = utf8_char_length(s);

        while(len != 0)
        {
            hash *= FNV_PRIME;
            hash ^= *s++;
            len--;
        }
    }

    return hash;
}

// -----------------------------------------------------------------------
// strip quotes off of parsed json token and recalculate hash

void strip_quotes(uint16_t len)
{
    uint16_t i;

    for(i = 0; i < len - 2; i++)
    {
        json_token[i] = json_token[i + 1];
    }

    json_token[i] = '\0';
    json_hash = fnv_hash(json_token);
}

// -----------------------------------------------------------------------
// copy next line of soruce out of json data into line_buff

static void refill(void)
{
    uint16_t i = 0;

    // line number only needed for error printf
    if(json_data[json_index] == 0x0a)
    {
        line_no++;
        json_index++;
    }

    while((json_index != json_len) &&
          (json_data[json_index] != 0x0a))
    {
        line_buff[i++] = json_data[json_index++];
        if(i == MAX_LINE_LEN)
        {
            json_error("JSON Line Too Long");
        }
    }

    line_left    = i;       // how much of current line is left to parse
    line_buff[i] = '\0';    // make line_buff asciiz
    line_index   = 0;       // reset current parse index
}

// -----------------------------------------------------------------------
// skip leasing white space prior to parsing next space delimited token

static void skip_white(void)
{
    char c;

    do
    {
        if(json_index == json_len)
        {
             return;        // out of data
        }

        // refill line buffer if it is empty
        if(line_left == 0)
        {
            refill();
        }

        // now scan past leading white space

        c = line_buff[line_index];
        while((c == 0x20) && (line_left != 0))
        {
            line_index++;
            line_left--;
            c = line_buff[line_index];
        }

        // not being able to comment in json is the single most
        // MORONIC decision they made.  there is zero reason
        // why a comment has to take up space in your structure
        // not like its that freeking difficult

        if((c == '/') && (line_left != 0))
        {
            if((line_buff[line_index + 1]) == '/')
            {
                line_left = 0;
            }
        }
    } while(line_left == 0);
}

// -----------------------------------------------------------------------
// convert all 0x09 and 0x0d chars into 0x20 (makes parsing easier)

void json_de_tab(char *s, size_t len)
{
    while(len != 0)
    {
        if((*s == 0x09) || (*s == 0x0d))
        {
            *s = 0x20;
        }
        s++; len--;
    }
}

// -----------------------------------------------------------------------
// extract next space delimited token from input data

void token(void)
{
    char *s = line_buff;
    uint16_t i, j = 0;
    uint8_t in_quotes = 0;
    uint8_t l;

    // if this call runs out of data before parsing a full token
    // then the hash value for what was parsed would be unknown
    // and json parsing will abort

    memset(json_token, 0, TOKEN_LEN);

    if(json_index == json_len)
    {
        return;
    }

    skip_white();           // skip leading white space

    // spaces inside quotes do not count as a token delimiter
    // this allows menu item names such as "Open File"

    while((line_left != 0) &&
          ((s[line_index] != 0x20) || (in_quotes != 0)))
    {
        // have we been fed a truncated utf8 character ?
        l = utf8_char_length(&s[line_index]);

        if(l > line_left)
        {
            // "warning: truncated utf8 character" ???
            line_left = 0;
            return;
        }

        if(s[line_index] == '"')
        {
            in_quotes ^= 1;
        }

       for(i = 0; i < l; i++)
       {
            json_token[j++] = s[line_index + i];

            if(j == TOKEN_LEN)
            {
                // error/warning token too long ?
                line_index += l;
                line_left  -= l;
                return;
            }
        }
        line_index += l;
        line_left  -= l;
    }
}

// =======================================================================
