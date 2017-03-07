/*
 * vertexQueue.c
 *
 * Implements a queue of vertices as a shallow graph. Convenience methods for doing this manually.
 */

#include <stddef.h>

#include "vertexQueue.h"


void addToVertexQueue(struct Vertex* v, struct ShallowGraph* queue, struct ShallowGraphPool* sgp) {
	struct VertexList* e = getVertexList(sgp->listPool);
	e->endPoint = v;
	appendEdge(queue, e);
}


struct Vertex* popFromVertexQueue(struct ShallowGraph* queue, struct ShallowGraphPool* sgp) {
	struct VertexList* e = popEdge(queue);
	if (e != NULL) {
		struct Vertex* v = e->endPoint;
		dumpVertexList(sgp->listPool, e);
		return v;
	} else {
		return NULL;
	}
}

struct Vertex* peekFromVertexQueue(struct ShallowGraph* queue) {
	struct VertexList* e = queue->edges;
	if (e != NULL) {
		return e->endPoint;
	} else {
		return NULL;
	}
}

