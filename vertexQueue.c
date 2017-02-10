/*
 * vertexQueue.c
 *
 * Implements a queue of vertices as a shallow graph. Convenience methods for doing this manually.
 */

#include "vertexQueue.h"


void addToVertexQueue(struct Vertex* v, struct ShallowGraph* queue, struct ShallowGraphPool* sgp) {
	struct VertexList* e = getVertexList(sgp->listPool);
	e->endPoint = v;
	appendEdge(queue, e);
}


struct Vertex* popFromVertexQueue(struct ShallowGraph* queue, struct ShallowGraphPool* sgp) {
	struct VertexList* e = popEdge(queue);
	struct Vertex* v = e->endPoint;
	dumpVertexList(sgp->listPool, e);
	return v;
}

