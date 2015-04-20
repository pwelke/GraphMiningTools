#include <malloc.h>

static int* pruning = NULL;
static int nPruning = 0;

static const int hashMod = sizeof(int) * 8;

void initPruning(const int nGraphs) {
	nPruning = nGraphs;
	if ((pruning = malloc(nGraphs * sizeof(int)))) {
		int i;
		for (i=0; i<nGraphs; ++i) {
			pruning[i] = 0;
		}
	}
}

void freePruning() {
	free(pruning);
}


int hashID(const int elementID) {
	return 1<<(elementID % (hashMod));
}

void addToPruningSet(const int elementID, const int index) {
	pruning[index] |= hashID(elementID);
}

void resetPruningSet(const int index) {
	pruning[index] = 0;
}

/* if the current index is larger than the pruning array aka. bloom filter array, 
double the size of the array. Then add the id hash to the filter. This method is 
useful for getVertexAndEdgeHistograms as we might not know the correct number of 
graphs in the database */
void initialAddToPruningSet(const int elementID, const int index) {
	if (index >= nPruning) {
		int i;
		pruning = realloc(pruning, 2 * nPruning * sizeof(int));
		for (i=nPruning; i<2*nPruning; ++i) {
			pruning[i] = 0;
		}
		nPruning *= 2;
	}
	addToPruningSet(elementID, index);
}

char containedInPruningSet(const int elementID, const int index) {
	return (pruning[index] & hashID(elementID)) != 0;
}

char isSubset(const int fingerPrint, const int index) {
	return ((pruning[index] & fingerPrint) == fingerPrint);
}

char isEmpty(const int index) {
	return pruning[index] == 0;
}