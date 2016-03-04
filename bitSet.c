#include <malloc.h>
#include <string.h>
#include "bitSet.h"

char* createBitset(size_t n) {
	char* bitset;
	if (n % 8 == 0) {
		bitset = calloc((n / 8), sizeof(char));
	} else {
		bitset = calloc((n / 8 + 1), sizeof(char));
	} 
	return bitset;
}

char* copyBitset(char* bitset, size_t n) {
	char* copy = createBitset(n);
	size_t nBytes = (n % 8 == 0) ? (n / 8) : (n / 8 + 1);
	memcpy(copy, bitset, nBytes);
	return copy;
}

void destroyBitset(char* bitset) {
	free(bitset);
}

void setBitTrue(char* bitset, size_t i) {
	bitset[i / 8] |= (1 << (i % 8));
}

void setBitFalse(char* bitset, size_t i) {
	bitset[i / 8] &= ~(1 << (i % 8));
}

void setBit(char* bitset, size_t i, char value) {
	if (value) {
		setBitTrue(bitset, i);
	} else {
		setBitFalse(bitset, i);
	}
}

char getBit(char* bitset, size_t i) {
	return (bitset[i / 8] & (1 << (i % 8))) ? 1 : 0;
}

void printBitset(char* bitset, size_t n, FILE* out) {
	fprintf(out, "[");
	for (size_t i=0; i<n; ++i) {
		fprintf(out, "%i", getBit(bitset, i));
	}
	fprintf(out, "]\n");
}

void bitsetUnion(char* a, char* b, size_t n) {
	int i = (n % 8) ? n / 8 : n / 8 - 1;
	for ( ; i>=0; --i) {
		a[i] |= b[i];
	}
}

void bitsetIntersection(char* a, char* b, size_t n) {
	int i = (n % 8) ? n / 8 : n / 8 - 1;
	for ( ; i>=0; --i) {
		a[i] &= b[i];
	}
}

// int main(int argc, char** argv) {
// 	char* bitset = createBitset(16);
// 	char* b = createBitset(16);
// 	char* c = createBitset(16);
// 	char* d;
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
