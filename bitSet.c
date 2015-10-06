#include <malloc.h>
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

// int main(int argc, char** argv) {
// 	char* bitset = createBitset(10);
// 	printBitset(bitset, 10, stdout);
// 	// printStrange(bitset, 2);
// 	setBitTrue(bitset, 5);
// 	setBitTrue(bitset, 8);
// 	printBitset(bitset, 10, stdout);
// 	setBitFalse(bitset, 5);
// 	printBitset(bitset, 10, stdout);

// 	destroyBitset(bitset);
// }
