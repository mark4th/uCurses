// json parsing - token extraction and hash calculation
// -----------------------------------------------------------------------

#include <stdlib.h>
#include <inttypes.h>
#include <aio.h>
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

    for(i = 0; i < len; i++)
    {
        json_token[i] = json_token[i + 1];
    }

    json_token[i] = '\0';
    json_hash = fnv_hash(json_token);
}

// -----------------------------------------------------------------------

static void refill(void)
{
    uint16_t i = 0;

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

    line_left    = i;
    line_buff[i] = '\0';
    line_index   = 0;
}

// -----------------------------------------------------------------------

static void skip_white(void)
{
    char c;

    do
    {
        if(json_index == json_len)
        {
             return;
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
        if(c == '#') { line_left = 0; }
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
    uint8_t l;

    // if this call runs out of data before parsing a full token
    // then the has value for what was parsed would be unknwon
    // and json parsing will abort

    memset(json_token, 0, TOKEN_LEN);

    if(json_index == json_len)
    {
        return;
    }

    skip_white();           // skip leading white space

    while((line_left != 0) &&
          (s[line_index] != 0x20))
    {
       // have we been fed a truncated utf8 character ?
       l = utf8_char_length(&s[line_index]);

       if(l > line_left)
       {
            // "warning: truncated utf8 character" ???
           line_left = 0;
           return;
       }

       for(i = 0; i < l; i++)
       {
           json_token[j++] = s[line_index + i];

           if(j == TOKEN_LEN)
           {
               return;
           }
       }
       line_index += l;
       line_left  -= l;
   }
}

// =======================================================================
