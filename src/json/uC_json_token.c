// json parsing - token extraction and hash calculation
// -----------------------------------------------------------------------

#include <inttypes.h>
#include <string.h>

#include <stdio.h>

#include "uCurses.h"
#include "uC_json.h"
#include "uC_utf8.h"
#include "uC_utils.h"

// -----------------------------------------------------------------------

extern json_vars_t *json_vars;

// -----------------------------------------------------------------------
// strip quotes off of parsed json token and recalculate hash

void strip_quotes(int16_t len)
{
    int16_t i;

    for (i = 0; i < len - 2; i++)
    {
        json_vars->json_token[i] =
        json_vars->json_token[i + 1];
    }

    json_vars->json_token[i] = '\0';

    json_vars->json_hash = fnv_hash(json_vars->json_token);
}

// -----------------------------------------------------------------------
// copy next line of soruce out of json data into line_buff

static void refill(void)
{
    int16_t i = 0;

    // line number only needed for error printf
    if (json_vars->json_data[json_vars->json_index] == 0x0a)
    {
        json_vars->line_no++;
        json_vars->json_index++;
    }

    while ((json_vars->json_index != json_vars->json_len) &&
           (json_vars->json_data[json_vars->json_index] != 0x0a))
    {
        json_vars->line_buff[i++] =
            json_vars->json_data[json_vars->json_index++];
        if (i == MAX_LINE_LEN)
        {
            json_error("JSON Line Too Long");
        }
    }

    json_vars->line_left    = i;       // how much of current line is left to parse
    json_vars->line_buff[i] = '\0';    // make line_buff asciiz
    json_vars->line_index   = 0;       // reset current parse index
}

// -----------------------------------------------------------------------
// skip leasing white space prior to parsing next space delimited token

static void skip_white(void)
{
    char c;

    do
    {
        // out of data?
        if (json_vars->json_index == json_vars->json_len)
        {
            return;
        }

        // refill line buffer if it is empty
        if (json_vars->line_left == 0)
        {
            refill();
        }

        // now scan past leading white space

        c = json_vars->line_buff[json_vars->line_index];

        while ((c == 0x20) && (json_vars->line_left != 0))
        {
            json_vars->line_index++;
            json_vars->line_left--;

            c = json_vars->line_buff[json_vars->line_index];
        }

        // not being able to comment in json is the single most
        // MORONIC decision they made.  there is zero reason
        // why a comment has to take up space in your structure
        // not like its that freeking difficult

        if ((c == '/') && (json_vars->line_left != 0))
        {
            if ((json_vars->line_buff[json_vars->line_index + 1]) == '/')
            {
                json_vars->line_left = 0;
            }
        }
    } while(json_vars->line_left == 0);
}

// -----------------------------------------------------------------------
// convert all 0x09 and 0x0d chars into 0x20 (makes parsing easier)

void json_de_tab(char *s, int len)
{
    while (len != 0)
    {
        if ((*s == 0x09) || (*s == 0x0d))
        {
            *s = 0x20;
        }
        s++;
        len--;
    }
}

// -----------------------------------------------------------------------
// extract next space delimited token from input data

void token(void)
{
    char *s = json_vars->line_buff;
    int16_t i, j = 0;
    int16_t in_quotes = 0;
    int16_t l;

    // if this call runs out of data before parsing a full token
    // then the hash value for what was parsed would be unknown
    // and json parsing will abort

    memset(json_vars->json_token, 0, MAX_TOKEN_LEN);

    if (json_vars->json_index == json_vars->json_len)
    {
        return;
    }

    skip_white(); // skip leading white space

    // spaces inside quotes do not count as a token delimiter
    // this allows menu item names such as "Open File"

    while ((json_vars->line_left != 0) &&
          ((s[json_vars->line_index] != 0x20) || (in_quotes != 0)))
    {
        // have we been fed a truncated utf8 character ?
        l = uC_utf8_char_length(&s[json_vars->line_index]);

        if (l > json_vars->line_left)
        {
            // "warning: truncated utf8 character" ???
            json_vars->line_left = 0;
            return;
        }

        if (s[json_vars->line_index] == '"')
        {
            in_quotes ^= 1;
        }

        for (i = 0; i < l; i++)
        {
            json_vars->json_token[j++] =
                s[json_vars->line_index + i];

            if (j == MAX_TOKEN_LEN)
            {
                break;
            }
        }
        json_vars->line_index += l;
        json_vars->line_left  -= l;
    }
}

// =======================================================================
