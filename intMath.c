#include "intMath.h"

/** 
Returns the smaller of the two input values
 */
int min(int a, int b) {
	return (a < b) ? a : b;
}

/*
 * returns the "correct" modulo of two numbers. b is assumed to be positive,
 * a is an arbitrary integer.
 */
int mod(int a, int b) {
	if (a % b >= 0) {
		return a % b;
	} else {
		return b + (a % b);
	}
}