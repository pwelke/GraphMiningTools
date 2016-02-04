#ifndef BIT_SET_H_
#define BIT_SET_H_ value

#include <stdio.h>

char* createBitset(size_t n);
char* copyBitset(char* bitset, size_t n);
void destroyBitset(char* bitset);

void setBitTrue(char* bitset, size_t i);
void setBitFalse(char* bitset, size_t i);
void setBit(char* bitset, size_t i, char value);

char getBit(char* bitset, size_t i);

void bitsetUnion(char* a, char* b, size_t n);
void bitsetIntersection(char* a, char* b, size_t n);

void printBitset(char* bitset, size_t n, FILE* out);

#endif
