#include "graph.h"
#include "stdio.h"

struct ShallowGraph* getComp(struct Vertex* v, int compNumber, struct ShallowGraph* comp, struct ListPool* lp) {
	struct VertexList* e;

	v->lowPoint = compNumber;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->lowPoint == -1) {
			pushEdge(comp, shallowCopyEdge(e, lp));
			getComp(e->endPoint, compNumber, comp, lp);
		} 
	}
	return comp;
}

void markComp(struct Vertex* v, int compNumber) {
	struct VertexList* e;

	v->lowPoint = compNumber;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->lowPoint == -1) {
			markComp(e->endPoint, compNumber);
		} 
	}
}

struct ShallowGraph* getConnectedComponents(struct Graph* g, struct ShallowGraphPool* sgp) {
	int v;
	int i = 0;
	struct ShallowGraph* result = NULL;

	for (v=0; v<g->n; ++v) {
		g->vertices[v]->lowPoint = -1;
	}

	for (v=0; v<g->n; ++v) {
		if (g->vertices[v]->lowPoint == -1) {
			struct ShallowGraph* comp = getComp(g->vertices[v], i, getShallowGraph(sgp), sgp->listPool);
			comp->next = result;
			result = comp;
			++i;
		}
	}
	return result;
}

/**
Returns a list of vertices who are roots of connected components. 
Result is a shallow graph. each edge->endPoint is pointing to a vertex
belonging to a unique connected component. */
struct ShallowGraph* getRepresentativeVertices(struct Graph* g, struct ShallowGraphPool* sgp) {
	int v;
	int i = 0;
	struct ShallowGraph* result = getShallowGraph(sgp);

	for (v=0; v<g->n; ++v) {
		g->vertices[v]->lowPoint = -1;
	}

	for (v=0; v<g->n; ++v) {
		if (g->vertices[v]->lowPoint == -1) {
			struct VertexList* e = getVertexList(sgp->listPool);
			pushEdge(result, e);
			e->endPoint = g->vertices[v];
			markComp(g->vertices[v], i);
			++i;
		}
	}
	return result;
}