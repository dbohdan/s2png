/*

This code is based on the implementation of RC4 created by Adam Back.
The original is available from http://www.cypherspace.org/adam/rsa/rc4c.html.

It is assumed to be in the Public Domain.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rc4.h"

#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t

void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key)
{
  int i;
  unsigned char t;
  unsigned char swapByte;
  unsigned char index1;
  unsigned char index2;
  unsigned char* state;
  short counter;

  state = &key->state[0];
  for(counter = 0; counter < 256; counter++)
  state[counter] = counter;
  key->x = 0;
  key->y = 0;
  index1 = 0;
  index2 = 0;
  for(counter = 0; counter < 256; counter++)
  {
    index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
    swap_byte(&state[counter], &state[index2]);
    index1 = (index1 + 1) % key_data_len;
  }
}

void drop_n(int n, rc4_key *key)
{
    if (n > 0)
    {
        unsigned char* temp = calloc(n, sizeof(unsigned char));
        rc4(temp, n, key);
        free(temp);
    }
}

void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key)
{
  unsigned char t;
  unsigned char x;
  unsigned char y;
  unsigned char* state;
  unsigned char xorIndex;
  short counter;

  x = key->x;
  y = key->y;
  state = &key->state[0];
  for(counter = 0; counter < buffer_len; counter++)
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

/*
#define buf_size 1024

int main(int argc, char* argv[])
{
  char seed[256];
  char data[512];
  char buf[buf_size];
  char digit[5];
  int hex, rd,i;
  int n;
  rc4_key key;

  if (argc < 2)
  {
    fprintf(stderr,"%s key <in >out\n",argv[0]);
    exit(1);
  }
  strcpy(data,argv[1]);
  n = strlen(data);
  if (n&1)
  {
    strcat(data,"0");
    n++;
  }
  n/=2;
  strcpy(digit,"AA");
  digit[4]='\0';
  for (i=0;i<n;i++)
  {
    digit[2] = data[i*2];
    digit[3] = data[i*2+1];
    sscanf(digit,"%x",&hex);
    seed[i] = hex;
  }

  prepare_key(seed,n,&key);
  rd = fread(buf,1,buf_size,stdin);
  while (rd>0)
  {
    rc4(buf,rd,&key);
    fwrite(buf,1,rd,stdout);
    rd = fread(buf,1,buf_size,stdin);
  }
}
*/
