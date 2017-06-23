#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "listSpanningTrees.h"
#include "listComponents.h"
#include "upperBoundsForSpanningTrees.h"
#include "connectedComponents.h"
#include "wilsonsAlgorithm.h"
#include "kruskalsAlgorithm.h"

#include "sampleSubtrees.h"

/**
Sample a spanning tree from a cactus graph, given as a list of its biconnected components, uniformly at random.
To this end, we just need to remove a random edge from each cycle = block of the graph. **/
struct ShallowGraph* sampleSpanningTreeEdgesFromCactus(struct ShallowGraph* biconnectedComponents, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTree = getShallowGraph(sgp);
	struct ShallowGraph* idx;
	for (idx=biconnectedComponents; idx!=NULL; idx=idx->next) {
		if (idx->m == 1) {
			appendEdge(spanningTree, shallowCopyEdge(idx->edges, sgp->listPool));
		} else {
			int removalEdgeId = rand() % idx->m;
			struct VertexList* e;
			int i = 0;
			for (e=idx->edges; e!=NULL; e=e->next) {
				if (i != removalEdgeId) {
					appendEdge(spanningTree, shallowCopyEdge(e, sgp->listPool));
				}
				++i;
			}
		}
	}
	return spanningTree;
}


/**
Sample a spanning tree from a cactus graph, given as a list of its biconnected components, uniformly at random.
To this end, we just need to remove a random edge from each cycle = block of the graph. **/
struct Graph* sampleSpanningTreeFromCactus(struct Graph* original, struct ShallowGraph* biconnectedComponents, struct GraphPool* gp) {
	struct Graph* spanningTree = emptyGraph(original, gp);
	struct ShallowGraph* idx;
	for (idx=biconnectedComponents; idx!=NULL; idx=idx->next) {
		if (idx->m == 1) {
			addEdgeBetweenVertices(idx->edges->startPoint->number, idx->edges->endPoint->number, idx->edges->label, spanningTree, gp);
		} else {
			int removalEdgeId = rand() % idx->m;
			struct VertexList* e;
			int i = 0;
			for (e=idx->edges; e!=NULL; e=e->next) {
				if (i != removalEdgeId) {
					addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, spanningTree, gp);
				}
				++i;
			}
		}
	}
	return spanningTree;
}


/**
Take k random spanning trees of g using Wilsons algorithm and return them as a list.
*/
struct ShallowGraph* sampleSpanningTreesUsingWilson(struct Graph* g, int k, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL;
	int j;
	for (j=0; j<k; ++j) {
		struct ShallowGraph* spanningTree = randomSpanningTreeAsShallowGraph(g, sgp);
		spanningTree->next = spanningTrees;
		spanningTrees = spanningTree;
	}
	return spanningTrees;
}

struct ShallowGraph* xsampleSpanningTreesUsingWilson(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)threshold;
	(void)gp;
	return sampleSpanningTreesUsingWilson(g, k, sgp);
}


/** from http://stackoverflow.com/questions/6127503/shuffle-array-in-c, 
but replaced their use of drand48 by rand
make sure you randomize properly using srand()! */
void shuffle(struct VertexList** array, size_t n) {
    // struct timeval tv;
    // gettimeofday(&tv, NULL);
    // int usec = tv.tv_usec;
    // srand48(usec);

    if (n > 1) {
        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int) (rand() % i);
            struct VertexList* t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}


struct ShallowGraph* sampleSpanningTreesUsingKruskalOnce(struct Graph* g, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTree = NULL;
	int i;

	// create and shuffle array of edges of g
	struct ShallowGraph* edges = getGraphEdges(g, sgp);
	struct VertexList** edgeArray = malloc(g->m * sizeof(struct VertexList*));
	if (edges->m != 0) {
		edgeArray[0] = edges->edges;
	}
	for (i=1; i < g->m; ++i) {
		edgeArray[i] = edgeArray[i-1]->next;
	}
	shuffle(edgeArray, g->m);

	// do the kruskal
	spanningTree = kruskalMST(g, edgeArray, gp, sgp);

	// garbage collection
	dumpShallowGraphCycle(sgp, edges);
	free(edgeArray);

	return spanningTree;
}

struct ShallowGraph* xsampleSpanningTreesUsingKruskalOnce(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)k;
	(void)threshold;
	return sampleSpanningTreesUsingKruskalOnce(g, gp, sgp);
}


struct ShallowGraph* sampleSpanningTreesUsingKruskal(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL;
	int i;

	for (i=0; i<k; ++i) {
		struct ShallowGraph* tmp = sampleSpanningTreesUsingKruskalOnce(g, gp, sgp);
		tmp->next = spanningTrees;
		spanningTrees = tmp;
	}
	return spanningTrees;
}


struct ShallowGraph* xsampleSpanningTreesUsingKruskal(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)threshold;
	return sampleSpanningTreesUsingKruskal(g, k, gp, sgp);
}


/**
List all spanning trees of g and draw k of them uniformly at random, return these k spanning trees as a list. 
*/
struct ShallowGraph* sampleSpanningTreesUsingListing(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL;
	struct ShallowGraph* trees = listSpanningTrees(g, sgp, gp);
	struct ShallowGraph** array;
	
	struct ShallowGraph* idx;
	int j;
	int nTrees = 0;

	/* find number of listed trees, put them in array */
	for (idx=trees; idx; idx=idx->next) ++nTrees;

	array = malloc(nTrees * sizeof(struct ShallowGraph*));
	j = 0;
	for (idx=trees; idx; idx=idx->next) {	
		array[j] = idx;
		++j;
	}

	/* sample k trees uniformly at random */
	for (j=0; j<k; ++j) {
		int rnd = rand() % nTrees;
		// can't just use the listed tree itself, as it might get selected more than once
		struct ShallowGraph* tree = cloneShallowGraph(array[rnd], sgp);
		tree->next = spanningTrees;
		spanningTrees = tree;
	}
	dumpShallowGraphCycle(sgp, trees);
	free(array);
	return spanningTrees;
}


struct ShallowGraph* xsampleSpanningTreesUsingListing(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)threshold;
	return sampleSpanningTreesUsingListing(g, k, gp, sgp);
}


/**
If there are expected to be less than threshold spanning trees, sample spanning trees using explicit listing, 
otherwise use wilsons algorithm.
*/
struct ShallowGraph* sampleSpanningTreesUsingMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	long upperBound = getGoodEstimate(g, sgp, gp);
	if ((upperBound < threshold) && (upperBound != -1)) {
		return sampleSpanningTreesUsingListing(g, k, gp, sgp);
	} else {
		return sampleSpanningTreesUsingWilson(g, k, sgp);
	}
}


struct ShallowGraph* xsampleSpanningTreesUsingMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	return sampleSpanningTreesUsingMix(g, k, threshold, gp, sgp);
}


/**
If there are expected to be less than threshold spanning trees, sample spanning trees using explicit listing, 
otherwise use wilsons algorithm.
*/
struct ShallowGraph* sampleSpanningTreesUsingPartialListingMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	long upperBound = getGoodEstimate(g, sgp, gp);

	if (k != 1) {
		fprintf(stderr, "This method will only sample one spanning tree, you asked for %i\n", k);
		k = 1;
	}
	if ((upperBound < threshold) && (upperBound != -1)) {
		int i = rand() % threshold;
		int storeI = i;
		struct ShallowGraph* garbage = listKSpanningTrees(g, &i, sgp, gp);
		if (i == 0) {
			struct ShallowGraph* result = garbage->prev;
			result->prev->next=garbage;
			dumpShallowGraphCycle(sgp, garbage);
			return result;
		} else {
			// TODO: do something sensible
			struct ShallowGraph* result = garbage->prev;
			result->prev->next=garbage;
			dumpShallowGraphCycle(sgp, garbage);
			fprintf(stderr, "oversampled! i=%i leftover=%i\n", i, storeI);
			return result;
		}
	} else {
		return sampleSpanningTreesUsingWilson(g, k, sgp);
	}
}


struct ShallowGraph* xsampleSpanningTreesUsingPartialListingMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	return sampleSpanningTreesUsingPartialListingMix(g, k, threshold, gp, sgp);
}


/**
If g is a cactus graph, use a specialized method to sample spanning trees, 
otherwise use sampleSpanningTreesUsingMix.
*/
struct ShallowGraph* sampleSpanningTreesUsingCactusMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL;
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	int blockCount = 0;
	struct ShallowGraph* idx;
	for (idx=biconnectedComponents; idx!=NULL; idx=idx->next) {
		if (idx->m > 1) {
			++blockCount;
		}
	}
	/* if g is a cactus graph */
	if (g->n - 1 + blockCount == g->m) {
		int j;
		for (j=0; j<k; ++j) {	
			struct ShallowGraph* spanningTree = sampleSpanningTreeEdgesFromCactus(biconnectedComponents, sgp);
			spanningTree->next = spanningTrees;
			spanningTrees = spanningTree;
		}
	} else {
		// for speedup. this is sampleSpanningTreesUsingMix
		long upperBound = getGoodEstimatePrecomputedBlocks(g, biconnectedComponents, sgp, gp);
		if ((upperBound < threshold) && (upperBound != -1)) {
			spanningTrees = sampleSpanningTreesUsingListing(g, k, gp, sgp);
		} else {
			spanningTrees = sampleSpanningTreesUsingWilson(g, k, sgp);
		}
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return spanningTrees;
}


struct ShallowGraph* xsampleSpanningTreesUsingCactusMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	return sampleSpanningTreesUsingCactusMix(g, k, threshold, gp, sgp);
}


/**
Return the list of trees in the bridge forest of g.
*/
struct ShallowGraph* listBridgeForest(struct Graph* g, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	// TODO replace this by a method that just creates the forest without listing cycles
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	struct ShallowGraph* bridgeTrees = getConnectedComponents(forest, sgp);
	struct ShallowGraph* tmp;
	// the resulting trees need to reference the original graph g
	for (tmp=bridgeTrees; tmp!=NULL; tmp=tmp->next) {
		rebaseShallowGraph(tmp, g);
	}
	/* garbage collection */
	dumpGraphList(gp, forest);
	return bridgeTrees;
}


struct ShallowGraph* xlistBridgeForest(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {	
	(void)k;
	(void)threshold;
	return listBridgeForest(g, gp, sgp);
}


/**
If there are expected to be less than threshold spanning trees, return a list containing all of them. 
Otherwise, sample k spanning trees using Wilsons algorithm. 
*/
struct ShallowGraph* listOrSampleSpanningTrees(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL; 
	long upperBound = getGoodEstimate(g, sgp, gp);
	if ((upperBound < threshold) && (upperBound != -1)) {
		spanningTrees = listSpanningTrees(g, sgp, gp);	
	} else {
		spanningTrees = sampleSpanningTreesUsingWilson(g, k, sgp);
	}
	return spanningTrees;
}


struct ShallowGraph* xlistOrSampleSpanningTrees(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	return listOrSampleSpanningTrees(g, k, threshold, gp, sgp);
}

struct ShallowGraph* runForEachConnectedComponent(struct ShallowGraph* (*sampler)(struct Graph*, int, long int, struct GraphPool*, struct ShallowGraphPool*), 
	struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* results = NULL;
	struct Graph* connectedComponents = listConnectedComponents(g, gp);

	struct Graph* current;
	for (current=connectedComponents; current!=NULL; current=current->next) {
		struct ShallowGraph* currentResult = sampler(current, k, threshold, gp, sgp);

		// find last element in currentResult
		struct ShallowGraph* idx;	
		for (idx=currentResult; idx->next!=NULL; idx=idx->next);

		// add previous results after last element, reset results to head of current results
		if (idx) {
			idx->next = results;
			results = currentResult;
		}
	}

//	TODO: BUG the following line would be necessary to avoid memory leaks, but the results shallow graphs point to the graphs in connectedComponents
//	dumpGraphList(gp, connectedComponents);
	return results;
}
