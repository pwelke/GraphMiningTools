#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "graph.h"
#include "searchTree.h"
#include "loading.h"
#include "listSpanningTrees.h"
#include "listComponents.h"
#include "outerplanar.h"
#include "upperBoundsForSpanningTrees.h"
#include "subtreeIsomorphism.h"
#include "graphPrinting.h"
#include "treeCenter.h"
#include "connectedComponents.h"
#include "cs_Tree.h"
#include "wilsonsAlgorithm.h"
#include "listCycles.h"
#include "cs_Cycle.h"


/**
Count the number of biconnected components that are not a bridge.
*/
int getNumberOfBlocks(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	int compNumber = 0;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			++compNumber;
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return compNumber;
}


/**
Count the number of edges in the graph that are bridges. 
I.e. count the number of biconnected components with only one edge.
*/
int getNumberOfBridges(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	int bridgeNumber = 0;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m == 1) {
			++bridgeNumber;
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return bridgeNumber;
}


/**
Count the number of connected components in the graph obtained from g by removing
all block edges (i.e. all edges that are not bridges)
*/
int getNumberOfBridgeTrees(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	int nConnectedComponents = listConnectedComponents(forest);
	dumpGraphList(gp, forest);
	return nConnectedComponents;
}


/**
Compute the maximum degree of any vertex in g.
A graph without vertices has maxdegree -1.

This method can handle graphs that are not full, i.e. there
are positions in the g->vertices array that are NULL.
*/
int getMaxDegree(struct Graph* g) {
	int max = -1;
	int v;
	for (v=0; v<g->n; ++v) {
		if (g->vertices[v] != NULL) {
			int deg = degree(g->vertices[v]);
			if (max < deg) {
				max = deg;
			}
		}
	}
	return max;
}


/**
Compute the minimum degree of any vertex in g.
A graph without vertices has mindegree INT_MAX.

This method can handle graphs that are not full, i.e. there
are positions in the g->vertices array that are NULL.
*/
int getMinDegree(struct Graph* g) {
	int min = INT_MAX;
	int v;
	for (v=0; v<g->n; ++v) {
		if (g->vertices[v] != NULL) {
			int deg = degree(g->vertices[v]);
			if (min > deg) {
				min = deg;
			}
		}
	}
	return min;
}

int* computeCycleDegrees(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);

	/* store for each vertex if the current bic.comp was already counted */
	int* occurrences = malloc(g->n * sizeof(int));
	/* store the cycle degrees of each vertex in g */
	int* cycleDegrees = malloc(g->n * sizeof(int));

	int v;
	struct ShallowGraph* comp;
	int compNumber = 0;

	for (v=0; v<g->n; ++v) {
		occurrences[v] = -1;
		cycleDegrees[v] = 0;
	}

	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			struct VertexList* e;
			for (e=comp->edges; e!=NULL; e=e->next) {
				if (occurrences[e->startPoint->number] < compNumber) {
					occurrences[e->startPoint->number] = compNumber;
					++cycleDegrees[e->startPoint->number];
				}
				if (occurrences[e->endPoint->number] < compNumber) {
					occurrences[e->endPoint->number] = compNumber;
					++cycleDegrees[e->endPoint->number];
				}
			}
			++compNumber;
		}			
	}
	free(occurrences);
	return cycleDegrees;
}


int getMaxCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp) {
	int maxDegree = -1;
	int* cycleDegrees = computeCycleDegrees(g, sgp);

	int v;
	for (v=0; v<g->n; ++v) {
		if (maxDegree < cycleDegrees[v]) {
			maxDegree = cycleDegrees[v];
		}
	}

	free(cycleDegrees);

	return maxDegree;
}


int getMinCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp) {
	int minDegree = INT_MAX;
	int* cycleDegrees = computeCycleDegrees(g, sgp);

	int v;
	for (v=0; v<g->n; ++v) {
		if (minDegree > cycleDegrees[v]) {
			minDegree = cycleDegrees[v];
		}
	}

	free(cycleDegrees);

	return minDegree;
}


int getNumberOfSimpleCycles(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct Graph* tmp;
	struct Graph* idx;
	int numCycles = 0;

	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	/* TODO refactor */
	struct Graph* biconnectedComponents = forest->next;

	/* list all cycles */
	struct ShallowGraph* simpleCycles = NULL;


	for (idx=biconnectedComponents; idx; idx=idx->next) {
		simpleCycles = addComponent(simpleCycles, listCycles(idx, sgp));
	}

	/* if cycles were found, compute canonical strings */
	if (simpleCycles) {
		struct ShallowGraph* cycle = NULL;

		/* transform cycle of shallow graphs to a list */
		simpleCycles->prev->next = NULL;
		simpleCycles->prev = NULL;

		for (cycle=simpleCycles; cycle!=NULL; cycle=cycle->next) {
			++numCycles;
		}

		dumpShallowGraphCycle(sgp, simpleCycles);
	} 

	/* garbage collection */
	for (idx=biconnectedComponents; idx; idx=tmp) {
		tmp = idx->next;
		dumpGraph(gp, idx);
	}
	dumpGraph(gp, forest);

	/* each cycle is found twice */
	return numCycles / 2;
}

int getNumberOfNonIsoCycles(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct Graph* tmp;
	struct Graph* idx;
	int numCycles;

	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	/* TODO refactor */
	struct Graph* biconnectedComponents = forest->next;

	/* list all cycles */
	struct ShallowGraph* simpleCycles = NULL;
	struct ShallowGraph* cyclePatterns = NULL;
	struct Vertex* cyclePatternSearchTree = NULL;

	for (idx=biconnectedComponents; idx; idx=idx->next) {
		simpleCycles = addComponent(simpleCycles, listCycles(idx, sgp));
	}

	/* if cycles were found, compute canonical strings */
	if (simpleCycles) {
		/* transform cycle of shallow graphs to a list */
		simpleCycles->prev->next = NULL;
		simpleCycles->prev = NULL;

		cyclePatterns = getCyclePatterns(simpleCycles, sgp);
		cyclePatternSearchTree = buildSearchTree(cyclePatterns, gp, sgp);

		numCycles = cyclePatternSearchTree->number;
	} else {
		numCycles = 0;
	}

	/* garbage collection */

	/* dump cycles, if any
	 * TODO may be moved upwards directly after finding simple cycles */
	if (cyclePatternSearchTree) {
			dumpSearchTree(gp, cyclePatternSearchTree);
	}

	/* dump biconnected components list */
	for (idx=biconnectedComponents; idx; idx=tmp) {
		tmp = idx->next;
		dumpGraph(gp, idx);
	}

	dumpGraph(gp, forest);

	/* each cycle is found twice */
	return numCycles / 2;
}


/**
A tree is a connected graph with m = n-1 edges.
*/
char isTree(struct Graph* g) {
	if (isConnected(g)) {
		return g->m == g->n - 1;
	} else { 
		return 0;
	}
}


/**
A cactus is a connected graph where each nontrivial biconnected block (i.e., a
biconnected component that is not a bridge) is a simple cycle.
*/
char isCactus(struct Graph* g, struct ShallowGraphPool* sgp) {
	if (isConnected(g)) {
		struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
		struct ShallowGraph* comp;
		int compNumber = 0;
		for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
			if (comp->m > 1) {
				++compNumber;
			}			
		}
		/* cleanup */
		dumpShallowGraphCycle(sgp, biconnectedComponents);
		return compNumber;
	} else {
		return 0;
	}
}


/**
An outerplanar graph is a graph that can be drawn in the plane such that
(1) edges only intersect at vertices and
(2) each vertex can be reached from the outer face without crossing an edge.

A graph is outerplanar if and only if each of its biconnected components is outerplanar.

TODO

outerplanar.[ch]

isMaximalOuterplanar() -> isOuterplanarBlock()
isOuterplanar() -> isOuterplanarBlockShallow()
isOuterplanarGraph() -> isOuterplanar()

*/ 
char isOuterplanarGraph(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	char isOp = 1;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			isOp = isOuterplanar(comp, sgp, gp);
			if (isOp == 0) {
				break;
			}
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return isOp;
}
