#include <malloc.h>
#include <string.h>
#include "bitSet.h"

uint8_t* createBitset(size_t n) {
	uint8_t* bitset;
	if (n % 8 == 0) {
		bitset = calloc((n / 8), sizeof(uint8_t));
	} else {
		bitset = calloc((n / 8 + 1), sizeof(uint8_t));
	} 
	return bitset;
}

uint8_t* copyBitset(uint8_t* bitset, size_t n) {
	uint8_t* copy = createBitset(n);
	size_t nBytes = (n % 8 == 0) ? (n / 8) : (n / 8 + 1);
	memcpy(copy, bitset, nBytes);
	return copy;
}

void destroyBitset(uint8_t* bitset) {
	free(bitset);
}

void setBitTrue(uint8_t* bitset, size_t i) {
	bitset[i / 8] |= (1 << (i % 8));
}

void setBitFalse(uint8_t* bitset, size_t i) {
	bitset[i / 8] &= ~(1 << (i % 8));
}

void setBit(uint8_t* bitset, size_t i, uint8_t value) {
	if (value) {
		setBitTrue(bitset, i);
	} else {
		setBitFalse(bitset, i);
	}
}

uint8_t getBit(uint8_t* bitset, size_t i) {
	return (bitset[i / 8] & (1 << (i % 8))) ? 1 : 0;
}

void printBitset(uint8_t* bitset, size_t n, FILE* out) {
	fprintf(out, "[");
	for (size_t i=0; i<n; ++i) {
		fprintf(out, "%i", getBit(bitset, i));
	}
	fprintf(out, "]\n");
}

void bitsetUnion(uint8_t* a, uint8_t* b, size_t n) {
	int i = (n % 8) ? n / 8 : n / 8 - 1;
	for ( ; i>=0; --i) {
		a[i] |= b[i];
	}
}

void bitsetIntersection(uint8_t* a, uint8_t* b, size_t n) {
	int i = (n % 8) ? n / 8 : n / 8 - 1;
	for ( ; i>=0; --i) {
		a[i] &= b[i];
	}
}

// int main(int argc, char** argv) {
//	 uint8_t* bitset = createBitset(16);
//	 uint8_t* b = createBitset(16);
//	 uint8_t* c = createBitset(16);
//	 uint8_t* d;
// 	printBitset(bitset, 16, stdout);
// 	// printStrange(bitset, 2);
// 	setBitTrue(bitset, 5);
// 	setBitTrue(bitset, 8);
// 	printBitset(bitset, 16, stdout);
// 	bitsetUnion(b, bitset, 16);
// 	printBitset(b, 16, stdout);
//
// 	setBitFalse(bitset, 5);
// 	printBitset(bitset, 16, stdout);
// 	bitsetUnion(c, bitset, 16);
// 	d = copyBitset(c, 16);
//
// 	printBitset(c, 16, stdout);
// 	printBitset(d, 16, stdout);
//
// 	bitsetUnion(c, b, 16);
// 	printBitset(c, 16, stdout);
//
// 	destroyBitset(bitset);
// 	destroyBitset(b);
// 	destroyBitset(c);
// }
