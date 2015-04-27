/** Compute the Hamiltonian path of a cactus graph or decide that there is none. */

#include <malloc.h>

#include "graph.h"
#include "listComponents.h"
#include "outerplanar.h"
#include "connectedComponents.h"
static int visit(struct Vertex* v, const int* criticality) {
	if (!(v->visited)) {
		v->visited = 1;
		if (criticality[v->number] > 1) {
			return 1;
		}
	}
	return 0;
}

/**
Check if a given connected cactus graph is traceable.
Assumes the input graph to be a connected cactus graph
*/
char isThisCactusTraceable(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	int* criticality = computeCriticality(biconnectedComponents, g->n);
	int v; 
	struct ShallowGraph* c;

	// test if every vertex has criticality at most 2
	for (v=0; v<g->n; ++v) {
		if (criticality[v] > 2) {
			free(criticality);
			dumpShallowGraphCycle(sgp, biconnectedComponents);
			return 0;
		}
	}

	// test if every biconnected component contains at most two critical vertices.
	for (c=biconnectedComponents; c!=NULL; c=c->next) {
		struct VertexList* e;
		int criticalVertexCount = 0;

		// TODO measure if the alg is faster with or without this line
		if (c->m == 1) continue;

		// init ->visited = 0
		for (e=c->edges; e!=NULL; e=e->next) {
			e->startPoint->visited = 0;
			e->endPoint->visited = 0;
		}
		// count critical vertices
		for (e=c->edges; e!=NULL; e=e->next) {
			criticalVertexCount += visit(e->startPoint, criticality);
			criticalVertexCount += visit(e->endPoint, criticality);
		}
		if (criticalVertexCount > 2) {
			free(criticality);
			dumpShallowGraphCycle(sgp, biconnectedComponents);
			return 0;
		 } else {
			if (criticalVertexCount == 2) {
				char notFound = 1;
				// search for the edge that contains both critical vertices
				for (e=c->edges; e!=NULL; e=e->next) {
					if ((criticality[e->startPoint->number] > 1) && (criticality[e->endPoint->number] > 1)) {
						notFound = 0;
						break;
					} 
				}
				if (notFound) {
					free(criticality);
					dumpShallowGraphCycle(sgp, biconnectedComponents);
					return 0;
				}
			}
		}
	}

	// // TODO is there a bug below? if there is only one critical vertex, then the method cannot find the edge its looking for

	// // test if the critical vertices of each block are adjacent
	// for (c=biconnectedComponents; c!=NULL; c=c->next) {
	// 	struct VertexList* e;
	// 	char notFound = 1;

	// 	// TODO measure if the alg is faster with or without this line
	// 	if (c->m == 1) continue;

	// 	// search for the edge that contains both critical vertices
	// 	for (e=c->edges; e!=NULL; e=e->next) {
	// 		if ((criticality[e->startPoint->number] > 1) && (criticality[e->endPoint->number] > 1)) {
	// 			notFound = 0;
	// 			break;
	// 		} 
	// 	}
	// 	if (notFound) {
	// 		free(criticality);
	// 		dumpShallowGraphCycle(sgp, biconnectedComponents);
	// 		return 0;
	// 	}
	// }

	// otherwise, there exists a Hamiltonian path
	return 1;
}

	
char isTraceableCactus(struct Graph* g, struct ShallowGraphPool* sgp) {
	return isCactus(g, sgp) && isThisCactusTraceable(g, sgp);
}


/**
Assumes the input graph to be a connected graph.
If this method returns 0, then the graph is not traceable.
If it returns 1, it might still not be traceable.
*/
char isWeaklyTraceableUnsafe(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	int* criticality = computeCriticality(biconnectedComponents, g->n);
	int v; 
	struct ShallowGraph* c;

	// test if every vertex has criticality at most 2
	for (v=0; v<g->n; ++v) {
		if (criticality[v] > 2) {
			free(criticality);
			dumpShallowGraphCycle(sgp, biconnectedComponents);
			return 0;
		}
	}

	// test if every biconnected component contains at most two critical vertices.
	for (c=biconnectedComponents; c!=NULL; c=c->next) {
		struct VertexList* e;
		int criticalVertexCount = 0;

		// TODO measure if the alg is faster with or without this line
		if (c->m == 1) continue;

		// init ->visited = 0
		for (e=c->edges; e!=NULL; e=e->next) {
			e->startPoint->visited = 0;
			e->endPoint->visited = 0;
		}
		// count critical vertices
		for (e=c->edges; e!=NULL; e=e->next) {
			criticalVertexCount += visit(e->startPoint, criticality);
			criticalVertexCount += visit(e->endPoint, criticality);
		}
		if (criticalVertexCount > 2) {
			free(criticality);
			dumpShallowGraphCycle(sgp, biconnectedComponents);
			return 0;
		 } else {
			/* this is where misclassification might occur.
			we would need to check, if there is a Hamiltonian path in the biconnected
			component starting at one critical vertex and stopping at the other, if 
			there is a Hamiltonian path starting at the single critical vertex, or if there 
			is no critical vertex, if the biconnected block is traceable in any way. */
		}
	}

	// otherwise, there might exist a Hamiltonian path
	return 1;
}

/**
If this method returns 0, then the graph is not traceable.
If it returns 1, it might still not be traceable.
*/
char isWeaklyTraceable(struct Graph* g, struct ShallowGraphPool* sgp) {
	return isConnected(g) && isWeaklyTraceableUnsafe(g, sgp);
}

