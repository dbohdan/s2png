/*
 *
 * This code is based on the implementation of RC4 created by John Allen.
 * The original is available from http://www.cypherspace.org/adam/rsa/rc4c.html.
 *
 * It is believed to be in the Public Domain.
 *
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rc4.h"

#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t

void prepare_key(uint8_t *key_data_ptr, size_t key_data_len,
                 struct rc4_key *key)
{
    uint8_t t;
    uint8_t index1;
    uint8_t index2;
    uint8_t* state;
    uint16_t counter;

    state = &key->state[0];
    for(counter = 0; counter < 256; counter++)
    state[counter] = counter;
    key->x = 0;
    key->y = 0;
    index1 = 0;
    index2 = 0;
    for (counter = 0; counter < 256; counter++)
    {
        index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
        swap_byte(&state[counter], &state[index2]);
        index1 = (index1 + 1) % key_data_len;
    }
}

void drop_n(size_t n, struct rc4_key *key)
{
    if (n > 0)
    {
        uint8_t* temp = calloc(n, sizeof(uint8_t));
        rc4(temp, n, key);
        free(temp);
    }
}

void rc4(uint8_t *buffer_ptr, size_t buffer_len, struct rc4_key *key)
{
    uint8_t t;
    uint8_t x;
    uint8_t y;
    uint8_t* state;
    uint8_t xorIndex;
    size_t counter;

    x = key->x;
    y = key->y;
    state = &key->state[0];
    for (counter = 0; counter < buffer_len; counter++)
    {
        x = (x + 1) % 256;
        y = (state[x] + y) % 256;
        swap_byte(&state[x], &state[y]);
        xorIndex = (state[x] + state[y]) % 256;
        buffer_ptr[counter] ^= state[xorIndex];
    }
    key->x = x;
    key->y = y;
}

bool pass_hash(char *indata, uint8_t *seed, size_t* outlen)
{
    char data[512];
    char digit[5];
    uint32_t hex;
    size_t i;
    size_t len;

    strncpy(data, indata, 512);
    data[512 - 1] = '\0';

    len = strlen(data);

    if (len & 1)
    {
        strcat(data, "0");
        len++;
    }

    len /= 2;

    strcpy(digit, "AA");
    digit[4] = '\0';

    for (i = 0; i < len; i++)
    {
        digit[2] = data[i * 2];
        digit[3] = data[i * 2 + 1];

        if (!isxdigit(digit[2]) || !isxdigit(digit[3]))
        {
            return false;
        }

        sscanf(digit, "%x", &hex);
        seed[i] = hex;
    }

    *outlen = len;

    return true;
}
