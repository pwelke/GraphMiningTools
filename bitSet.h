#ifndef BIT_SET_H_
#define BIT_SET_H_ value

#include <stdio.h>

char* createBitset(int n);
void destroyBitset(char* bitset);

void setBitTrue(char* bitset, int n);
void setBitFalse(char* bitset, int n);
void setBit(char* bitset, int n, char value);

char getBit(char* bitset, int n);

void printBitset(char* bitset, int size, FILE* out);

#endif
