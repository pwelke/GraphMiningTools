/**
 * Implementation of an algorithm to find a minimum number of paths covering all vertices in a poset.
 */

#include <stddef.h>
#include <limits.h>

#include "vertexQueue.h"

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



struct Graph* createVertexCoverOfPoset(struct Graph* g, struct GraphPool* gp) {
	struct Graph* flowInstance = createGraph(2 * g->n, gp);

	// source and sink of s-t flow instance
	struct Vertex* s = flowInstance->vertices[0];
	struct Vertex* t = flowInstance->vertices[1];

	// actual elements of poset start at vertex 1. they have infinite capacity.
	// HINT: if you ever want to change this for general graphs, consider that the
	// above s,t positions and size of flowInstance need to be changed.
	for (int v=1; v<g->n; ++v) {
		struct Vertex* vStart = flowInstance->vertices[2 * v];
		struct Vertex* vEnd = flowInstance->vertices[2 * v + 1];
		// add v -> v' edge
		struct VertexList* vertexEdge = getVertexList(gp->listPool);
		vertexEdge->startPoint = vStart;
		vertexEdge->endPoint = vEnd;
		vertexEdge->flag = INT_MAX;
		addEdge(vStart, vertexEdge);

		// add copy of original edge, they have infinite capacity
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			struct VertexList* edgeEdge = getVertexList(gp->listPool);
			edgeEdge->startPoint = vEnd;
			edgeEdge->endPoint = flowInstance->vertices[2 * e->endPoint->number];
			edgeEdge->flag = INT_MAX;
			addEdge(vEnd, edgeEdge);
		}

		// add edge from s to vEnd with capacity 1
		struct VertexList* newEdge = getVertexList(gp->listPool);
		newEdge->startPoint = s;
		newEdge->endPoint = vEnd;
		newEdge->flag = 1;
		addEdge(s, newEdge);

		// add edge from v to t
		newEdge = getVertexList(gp->listPool);
		newEdge->startPoint = vStart;
		newEdge->endPoint = t;
		newEdge->flag = 1;
		addEdge(vStart, newEdge);
	}

	return flowInstance;
}



