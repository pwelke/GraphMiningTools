#ifndef BIT_SET_H_
#define BIT_SET_H_ value

#include <stdio.h>
#include <stdint.h>

uint8_t* createBitset(size_t n);
uint8_t* copyBitset(uint8_t* bitset, size_t n);
void destroyBitset(uint8_t* bitset);

void setBitTrue(uint8_t* bitset, size_t i);
void setBitFalse(uint8_t* bitset, size_t i);
void setBit(uint8_t* bitset, size_t i, uint8_t value);

uint8_t getBit(uint8_t* bitset, size_t i);

void bitsetUnion(uint8_t* a, uint8_t* b, size_t n);
void bitsetIntersection(uint8_t* a, uint8_t* b, size_t n);

void printBitset(uint8_t* bitset, size_t n, FILE* out);

#endif
