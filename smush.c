// smush.c  - very cheezy compression used in the usage display code
// ------------------------------------------------------------------------

// copyright (c) 2021 MM :)

// ------------------------------------------------------------------------

int count = 0;
uint16_t blob;
int nblob;

// ------------------------------------------------------------------------

void emit(uint16_t c)
{
    printf("0x%04x, ", c);
    count++;

    if ((count % 8) == 0)
    {
        printf("\n");
    }
}

// ------------------------------------------------------------------------

void smush(uint8_t c)
{
    blob <<= 5;
    blob += c;
    nblob++;

    if (nblob == 3)
    {
        emit(blob);
        blob = 0;
        nblob = 0;
    }
}

// -----------------------------------------------------------------------

void smusher(uint8_t c)
{
    int i;

    uint8_t xlat[] =
    {
        0x00, 0x09, 0x0a, 0x21, 0x22, 0x28, 0x29, 0x2c,
        0x2d, 0x2e, 0x2f, 0x3a, 0x43, 0x46, 0x47, 0x48,
        0x49, 0x53, 0x54, 0x59, 0x5b, 0x5d, 0x5f
    };

    if (c == 0x20)
    {
        smush(0);
        return;
    }

    for (i = 0; i < sizeof(xlat); i++)
    {
        if(c == xlat[i])
        {
            smush(0x1e);
            smush(i);
            return;
        }
    }
    exit(1);
}

// -----------------------------------------------------------------------

void smush_loop(void)
{
    printf("0x%04x,\n", (unsigned int)strlen(text));
    int count = 0;

    for (i = 0; i < strlen(message); i++)
    {
        c = message[i];
        if (c < 0x61)
        {
            smusher(c);
        }
        else
        {
            c -= 0x60;
            smush(c);
        }
    }

    while(nblob != 0)
    {
        smush(0);
    }
    printf("\n");
}

// =======================================================================
