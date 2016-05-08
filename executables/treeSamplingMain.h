#ifndef TREE_SAMPLING_MAIN
#define TREE_SAMPLING_MAIN 

char DEBUG_INFO = 1;

typedef enum {
		wilson,
		kruskal,
		listing,
		mix,
		partialListing,
		cactusSampling,
		bridgeForest,
		listOrSample,
	} SamplingMethod;	

typedef enum {
		cs,
		fo,
		tr,
		mi
} OutputMethod;

int main(int argc, char** argv);

#endif
