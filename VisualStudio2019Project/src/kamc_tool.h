#pragma once
#pragma warning(disable: 4996)

#define MAX_SIZE (512*1024)

int lz_decode(unsigned char* in, int* isize, unsigned char* out, int* osize);
int lz_encode(unsigned char* in, int* isize, unsigned char* out, int* osize);
