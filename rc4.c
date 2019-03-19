/*
 * This code is based on the implementation of RC4 created by John Allen.
 * The original is available from http://www.cypherspace.org/adam/rsa/rc4c.html.
 * The original is believed to be in the public domain.
 * Modifications by dbohdan are released to the public domain.
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

void prepare_key(
    uint8_t *seed_data_ptr,
    size_t seed_data_len,
    struct rc4_key *key_ptr
)
{
    uint8_t t;
    uint8_t index1;
    uint8_t index2;
    uint8_t* state;
    uint16_t counter;

    state = &key_ptr->state[0];
    for(counter = 0; counter < 256; counter++)
    state[counter] = counter;
    key_ptr->x = 0;
    key_ptr->y = 0;
    index1 = 0;
    index2 = 0;
    for (counter = 0; counter < 256; counter++)
    {
        index2 = (seed_data_ptr[index1] + state[counter] + index2) % 256;
        swap_byte(&state[counter], &state[index2]);
        index1 = (index1 + 1) % seed_data_len;
    }
}

void drop_n(size_t n, struct rc4_key *key_ptr)
{
    if (n > 0)
    {
        uint8_t* temp = calloc(n, sizeof(uint8_t));
        rc4(temp, n, key_ptr);
        free(temp);
    }
}

void rc4(uint8_t *buffer_ptr, size_t buffer_len, struct rc4_key *key_ptr)
{
    uint8_t t;
    uint8_t x;
    uint8_t y;
    uint8_t* state;
    uint8_t xor_index;
    size_t counter;

    x = key_ptr->x;
    y = key_ptr->y;
    state = &key_ptr->state[0];
    for (counter = 0; counter < buffer_len; counter++)
    {
        x = (x + 1) % 256;
        y = (state[x] + y) % 256;
        swap_byte(&state[x], &state[y]);
        xor_index = (state[x] + state[y]) % 256;
        buffer_ptr[counter] ^= state[xor_index];
    }
    key_ptr->x = x;
    key_ptr->y = y;
}

bool pass_scan(
    char *hex_pass_ptr,
    uint8_t *seed_data_ptr,
    size_t* seed_data_len_ptr
)
{
    char data[512];
    char digit[5];
    uint32_t hex;
    size_t i;
    size_t len;

    strncpy(data, hex_pass_ptr, 512);
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
        seed_data_ptr[i] = hex;
    }

    *seed_data_len_ptr = len;

    return true;
}
