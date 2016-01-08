#ifndef __RC4_H_INCLUDED
#define __RC4_H_INCLUDED

struct rc4_key
{
    uint8_t state[256];
    uint8_t x;
    uint8_t y;
};

void prepare_key(uint8_t *key_data_ptr, size_t key_data_len,
                 struct rc4_key *key);
void rc4(uint8_t *buffer_ptr, size_t buffer_len, struct rc4_key *key);
void drop_n(size_t n, struct rc4_key *key);
size_t pass_hash(char* indata, uint8_t *seed);

#endif
