/*
 * This code is based on the implementation of RC4 created by John Allen.
 * The original is available from http://www.cypherspace.org/adam/rsa/rc4c.html.
 * The original is believed to be in the public domain.
 * Modifications by dbohdan are released to the public domain.
 *
 */

#ifndef __RC4_H_INCLUDED
#define __RC4_H_INCLUDED

struct rc4_key
{
    uint8_t state[256];
    uint8_t x;
    uint8_t y;
};

void prepare_key(
    uint8_t *seed_data_ptr,
    size_t seed_data_len,
    struct rc4_key *key_ptr
);
void rc4(
    uint8_t *buffer_ptr,
    size_t buffer_len,
    struct rc4_key *key_ptr
);
void drop_n(
    size_t n,
    struct rc4_key *key_ptr
);
bool pass_scan(
    char* hex_pass,
    uint8_t *seed_data_ptr,
    size_t* seed_data_len_ptr
);
#endif
