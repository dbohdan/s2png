#ifndef __RC4_H_INCLUDED
#define __RC4_H_INCLUDED

struct rc4_key
{
    unsigned char state[256];
    unsigned char x;
    unsigned char y;
};

void prepare_key(unsigned char *key_data_ptr, int key_data_len,
                 struct rc4_key *key);
void rc4(unsigned char *buffer_ptr, int buffer_len, struct rc4_key *key);
void drop_n(int n, struct rc4_key *key);
int pass_hash(char* indata, unsigned char *seed);

#endif
