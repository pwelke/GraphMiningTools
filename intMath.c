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

int pow(int b, int e){
    //negative sucks just treat it as zero.
    int r;
    if(e<=0) return 1;
    if(e%2==0){
        r=pow(b,e/2);
        return r*r;
    }
    return b*pow(b,e-1);
}
