#include <malloc.h>
#include <string.h>
#include "bitSet.h"

char* createBitset(int n) {
	int i;
	char* bitset;
	if (n % 8 == 0) {
		bitset = malloc((n / 8) * sizeof(char));
		for (i=0; i<n%8; ++i) {
			bitset[i] = 0;
		}
	} else {
		bitset = malloc((n / 8 + 1) * sizeof(char));	
		for (i=0; i<n % 8 + 1; ++i) {
			bitset[i] = 0;
		}
	} 
	return bitset;
}

char* copyBitset(char* bitset, int n) {
	char* copy = createBitset(n);
	size_t nBytes = (n % 8 == 0) ? (n / 8) : (n / 8 + 1);
	memcpy(copy, bitset, nBytes);
	return copy;
}

void destroyBitset(char* bitset) {
	free(bitset);
}

void setBitTrue(char* bitset, int n) {
	bitset[n / 8] |= (1 << (n % 8));
}

void setBitFalse(char* bitset, int n) {
	bitset[n / 8] &= ~(1 << (n % 8));
}

void setBit(char* bitset, int n, char value) {
	if (value) {
		setBitTrue(bitset, n);
	} else {
		setBitFalse(bitset, n);
	}
}

char getBit(char* bitset, int n) {
	return (bitset[n / 8] & (1 << (n % 8))) ? 1 : 0;
}

void printBitset(char* bitset, int size, FILE* out) {
	int i;
	fprintf(out, "[");
	for (i=0; i<size; ++i) {
		fprintf(out, "%i", getBit(bitset, i));
	}
	fprintf(out, "]\n");
}

void bitsetUnion(char* a, char* b, int n) {
	int i = (n % 8) ? n / 8 : n / 8 - 1;
	for ( ; i>=0; --i) {
		a[i] |= b[i];
	}
}

void bitsetIntersection(char* a, char* b, int n) {
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
	
// 	setBitFalse(bitset, 5);
// 	printBitset(bitset, 16, stdout);
// 	bitsetUnion(c, bitset, 16);
// 	d = copyBitset(c, 16);

// 	printBitset(c, 16, stdout);
// 	printBitset(d, 16, stdout);
	
// 	bitsetUnion(c, b, 16);
// 	printBitset(c, 16, stdout);

// 	destroyBitset(bitset);
// 	destroyBitset(b);
// 	destroyBitset(c);
// }
