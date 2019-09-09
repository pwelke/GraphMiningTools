#include <limits.h>

#include "intMath.h"

/** 
Returns the smaller of the two input values
 */
inline int min(int a, int b) {
	return (a < b) ? a : b;
}

/** 
Returns the larger of the two input values
 */
inline int max(int a, int b) {
    return (a > b) ? a : b;
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

int ipow(int b, int e){
    //negative sucks just treat it as zero.
    int r;
    if(e<=0) return 1;
    if(e%2==0){
        r=ipow(b,e/2);
        return r*r;
    }
    return b*ipow(b,e-1);
}


char intMultiplicationWillOverflow(int a, int x) {
	return     (a > INT_MAX / x) /* `a * x` would overflow */
			|| (a < INT_MIN / x) /* `a * x` would underflow */
			|| ((a == -1) && (x == INT_MIN)) /* `a * x` can overflow */;
}


char longMultiplicationWillOverflow(long int a, long int x) {
	return     (a > LONG_MAX / x) /* `a * x` would overflow */
			|| (a < LONG_MIN / x) /* `a * x` would underflow */
			|| ((a == -1) && (x == LONG_MIN)) /* `a * x` can overflow */;
}
