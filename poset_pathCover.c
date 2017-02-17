/**
 * Implementation of an algorithm to find a minimum number of paths covering all vertices in a poset.
 */

#include <malloc.h>
#include <stddef.h>
#include <limits.h>

#include "vertexQueue.h"
#include "bipartiteMatching.h"


/**
 * Count the number of reachable vertices.
 *
 * Assumes that the ->visited flags of all reachable vertices can be set to v->number to mark visited vertices.
 */
int numberOfReachableVertices(struct Vertex* v, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* queue = getShallowGraph(sgp);
	int markerID = v->number;
	addToVertexQueue(v, queue, sgp);
	v->visited = markerID;

	int reachableCount = 1;

	for (struct Vertex* w=popFromVertexQueue(queue, sgp); w!=NULL; w=popFromVertexQueue(queue, sgp)) {
		for (struct VertexList* e=w->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->visited != markerID) {
				e->endPoint->visited = markerID;
				addToVertexQueue(e->endPoint, queue, sgp);
				++reachableCount;
			}
		}
	}

	return reachableCount;
}


/**
 * Compute the number of reachable vertices for all vertices in g and store the value in ->lowPoint.
 * Uses ->visited flags of vertices. Does not clean up.
 */
void computeAllReachabilityCounts(struct Graph* g, struct ShallowGraphPool* sgp) {
	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->visited = -1;
	}

	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->lowPoint = numberOfReachableVertices(g->vertices[v], sgp);
	}
}



static struct Graph* createVertexCoverFlowInstanceOfPoset(struct Graph* g, struct GraphPool* gp) {
	struct Graph* flowInstance = createGraph(2 * g->n, gp);
	int maxCapacity = g->n;

	// source and sink of s-t flow instance
	struct Vertex* s = flowInstance->vertices[0];
	struct Vertex* t = flowInstance->vertices[1];

	// actual elements of poset start at vertex 1. they have infinite capacity.
	// HINT: if you ever want to change this for general graphs, consider that the
	// above s,t positions and size of flowInstance need to be changed.
	for (int v=1; v<g->n; ++v) {
		struct Vertex* vStart = flowInstance->vertices[2 * v];
		struct Vertex* vEnd = flowInstance->vertices[2 * v + 1];

		// add edge for each vertex in the original graph
		addResidualEdgesWithCapacity(vStart, vEnd, maxCapacity, gp->listPool);

		// add copy of original edges and residuals, they have infinite capacity
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			addResidualEdgesWithCapacity(vEnd, flowInstance->vertices[2 * e->endPoint->number], maxCapacity, gp->listPool);
		}

		// add edge from s to vEnd with capacity 1
		addResidualEdgesWithCapacity(s, vEnd, 1, gp->listPool);

		// add edge from v to t
		addResidualEdgesWithCapacity(vStart, t, 1, gp->listPool);
	}

	return flowInstance;
}


static int* getPathInFlowInstance(struct Vertex* v) {

	// DFS without backtracking in DAG
	int pathLength = 2;
	for (struct VertexList* e=v->neighborhood; e!=NULL && (e->flag > 0); e=e->endPoint->neighborhood) {
		++pathLength;
	}

	// debug
	if (pathLength % 2 == 1) {
		fprintf(stderr, "Pathlength is odd. This must not happen.\n");
	}

	int* path = malloc((pathLength / 2) * sizeof(int));
	path[0] = pathLength / 2;
	path[1] = v->number;

	pathLength = 2;
	// DFS without backtracking in DAG
	--v->visited;
	for (struct VertexList* e=v->neighborhood; e!=NULL && (e->flag > 0); e=e->endPoint->neighborhood) {
		--e->flag;
		--e->endPoint->visited;
		path[pathLength] = e->endPoint->number;
		++pathLength;
	}

	return path;
}

//static int* getLongestPathInFlowInstance(struct Vertex* v, struct Graph* poset, struct ShallowGraphPool* sgp) {
//
//	struct ShallowGraph* border = getShallowGraph(sgp);
//	addToVertexQueue(v, border, sgp);
//	v->lowPoint = -1;
//	v->d = 1;
//
//	// use ->lowPoint to store the a parent of w
//	// for sake of simplicity we count the number of times a vertex is in the border and
//	struct Vertex* highestVertex = v;
//	for (struct Vertex* w=popFromVertexQueue(border, sgp); w!=NULL;  w=popFromVertexQueue(border, sgp)) {
//		highestVertex = w;
//		w->d -= 1;
//		if (w->d == 0) {
//			for (struct VertexList* e=w->neighborhood; e!=NULL; e=e->next) {
//				if ((e->endPoint->visited == 0) && (e->endPoint->d == 0)) {
//					e->endPoint->lowPoint = w->number;
//					e->endPoint->d += 1;
//					addToVertexQueue(e->endPoint, border, sgp);
//				}
//			}
//		}
//	}
//	// here w stores the last visited pattern that, by construction, is on the highest level of the poset.
//	// backtrack using the ->lowPoints from here.
//	int pathLength = 2;
//	int* path = NULL;
//	for (struct Vertex* x=highestVertex; x->lowPoint!=-1; x=poset->vertices[x->lowPoint]) {
//		++pathLength;
//	}
//
//	path = malloc(pathLength * sizeof(int));
//	path[0] = pathLength;
//	path[1] = v->number;
//
//	pathLength -= 1;
//	for (struct Vertex* x=highestVertex; x->lowPoint!=-1; x=poset->vertices[x->lowPoint]) {
//		path[pathLength] = x->number;
//		--pathLength;
//	}
//
//	return path;
//}



/**
 *
 * Assumes that edges in the original poset are guaranteed to go from smaller vertex->number to larger vertex->number.
 * Also assumes that the poset starts with an artificial vertex at position 0 that points to the smallest elements, but
 * is not a part of the poset.
 */
int** getPathCoverOfPoset(struct Graph* g, int* nPaths, struct GraphPool* gp) {

	struct Graph* flowInstance = createVertexCoverFlowInstanceOfPoset(g, gp);

	// find max number of s-t paths
	int nAugmentations = 0;
	while (augmentWithCapacity(flowInstance->vertices[0], flowInstance->vertices[1])) {
		++nAugmentations;
	}

	// add one flow value to those edges that correspond to vertices
	for (int v=2; v<flowInstance->n; v+=2) {
		for (struct VertexList* e=flowInstance->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->number == v+1) {
				e->flag += 1;
				break; // iterating over the neighbors
			}
		}
	}

	// remove all edges that have 0 flow, all residual edges (assumed to go from larger vertex id to smaller),
	// and all edges containing s and t
	// init ->visited of vertices
	dumpVertexList(gp->listPool, flowInstance->vertices[0]->neighborhood);
	dumpVertexList(gp->listPool, flowInstance->vertices[1]->neighborhood);
	for (int v=2; v<flowInstance->n; ++v) {
		struct VertexList* keep = NULL;
		flowInstance->vertices[v]->visited = 0;
		for (struct VertexList* e=flowInstance->vertices[v]->neighborhood; e!=NULL; e=flowInstance->vertices[v]->neighborhood) {
			flowInstance->vertices[v]->neighborhood = e->next;
			if ((e->flag != 0) || (e->startPoint->number > e->endPoint->number)) {
				e->next = keep;
				keep = e;
			} else {
				dumpVertexList(gp->listPool, e);
			}
		}
		flowInstance->vertices[v]->neighborhood = keep;
	}

	// store outdegree - indegree in v->visited
	for (int v=2; v<flowInstance->n; ++v) {
		for (struct VertexList* e=flowInstance->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			e->startPoint->visited += 1;
			e->endPoint->visited -= 1;
		}
	}

	// how many paths are there?
	*nPaths = 0;
	for (int v=2; v<flowInstance->n; ++v) {
		if (flowInstance->vertices[v]->visited > 0) {
			*nPaths += flowInstance->vertices[v]->visited;
		}
	}

	// find paths from sources to sinks and store them in output
	int** paths = malloc(*nPaths * sizeof(int*));
	for (int v=2, i=0; v<flowInstance->n; ++v) {
		while (flowInstance->vertices[v]->visited > 0) {
			paths[i] = getPathInFlowInstance(flowInstance->vertices[v]);
			++i;
		}
	}

	dumpGraph(gp, flowInstance);
	return paths;
}



