#include <malloc.h>

#include "graph.h"
#include "outerplanar.h"
#include "dfs.h"

//debug
#include "graphPrinting.h"


/* return n choose r */
long int nCr(long int n, long int r) {
	long int numer = 1;
	long int denom = 1;
	int i;
	if (n - r < r) {
		r = n - r;
	}
	if (r == 0) {
		return 1;
	}
	for (i=n; i>n-r; --i) {
		numer *= i;
	}
	for (i=1; i<r+1; ++i) {
		denom *= i;
	}
	return numer / denom;
}

long int trivialBound(int m, int n) {
	return nCr(m, n-1);
}

long int outerplanarBound(long int m, long int n) {
	double estimate = 0;
	int i;
	long int k = m-n;
	for (i=0; i<k+1; ++i) {
		double mean = n / (double)(i+1);
		double meanPot = 1;
		int j;
		for (j=0; j<i+1; ++j) {
			meanPot *= mean;
		}
		estimate += nCr(k, i) * meanPot;
	}
	return (long int)estimate;
}


/** 
Count the number of distinct vertices that are incident to an edge
in g. n is the number of vertices in the graph from which g was obtained. 
vertices is an int array of size n.
*/
int getVertexNumberOfInducedGraph(struct ShallowGraph* g, int n, int* vertices) {
	int i;
	struct VertexList* e;
	int nInd = 0;
	for(i=0; i<n; ++i) { vertices[i] = 0; }
	for(e=g->edges; e!=NULL; e=e->next) {
		vertices[e->startPoint->number] = 1;
		vertices[e->endPoint->number] = 1;
	}
	for (i=0; i<n; ++i) {
		if (vertices[i] != 0) {
			++nInd;
		}
	}
	return nInd;
}


/** lots of stuff remain to do **/
long int exactNumberOfSpanningTrees(struct ShallowGraph* g, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	// debug
	struct Graph* comp = shallowGraphToGraph(g, gp);
	struct ShallowGraph* hamiltonianCycle = getCycleAndDiagonals(comp, sgp);
	struct ShallowGraph* diagonals = hamiltonianCycle->next;
	struct VertexList* e;

	for (e=diagonals->edges; e!=NULL; e=e->next) {
		e->flag = 0;
	}

	dumpShallowGraphCycle(sgp, hamiltonianCycle);
	dumpGraph(gp, comp);

	return -1;
}


/**
Return an upper bound on the number of spanning trees in g.
Multiplies upper bounds for each biconnected component.
For a bridge, there is exactly one spanning tree, 
for a general biconnected block with n vertices and m edges 
an upper bound is m choose (n-1). For outerplanar blocks there
is a better estimate than that, which will be applied.  
*/
long int getGoodEstimate(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* biconnectedComponents = findBiconnectedComponents(g, sgp);
	struct ShallowGraph* idx;
	long int estimate = 1;
	int* vertices = malloc(g->n * sizeof(int));

	for (idx=biconnectedComponents; idx!=NULL; idx=idx->next) {
		if (idx->m != 1) {
			int nInd = getVertexNumberOfInducedGraph(idx, g->n, vertices);
			if (!isOuterplanar(idx, sgp, gp)) {
				estimate *= trivialBound(idx->m, nInd);
			} else {
				estimate *= outerplanarBound(idx->m, nInd);
			}
		} else {
			/* Biconnected Component idx is a bridge. Computation would be:
			estimate *= 1;
			However, we do not want to waste time doing nothing */
		}
	}

	dumpShallowGraphCycle(sgp, biconnectedComponents);
	free(vertices);
	return estimate;
}


long int getEstimate(struct Graph* g, long int estimateFunction(long int, long int), char checkOuterplanarity, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* biconnectedComponents = findBiconnectedComponents(g, sgp);
	struct ShallowGraph* idx;
	long int estimate = 1;
	int* vertices = malloc(g->n * sizeof(int));

	for (idx=biconnectedComponents; idx!=NULL; idx=idx->next) {
		if (idx->m != 1) {
			int nInd;
			if (checkOuterplanarity) {
				if (!isOuterplanar(idx, sgp, gp)) {
					dumpShallowGraphCycle(sgp, biconnectedComponents);
					return -1;
				} else {
					
				}

			}
			nInd = getVertexNumberOfInducedGraph(idx, g->n, vertices);
			/* printf("(%li %i %i) ", estimateFunction(idx->m, nInd), nInd, idx->m); */
			estimate *= estimateFunction(idx->m, nInd);
		}
	}

	dumpShallowGraphCycle(sgp, biconnectedComponents);
	free(vertices);
	return estimate;
}