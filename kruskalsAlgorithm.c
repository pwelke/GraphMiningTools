#include <stdlib.h>

#include "graph.h"

static struct Vertex* getRoot(struct Vertex* v) {
	struct Vertex* r = v;
	while (r->neighborhood != NULL) {
		r = r->neighborhood->endPoint; 
	}
	return r;
}

static char isCycleFree(struct Graph* branching, struct VertexList* e, struct ListPool* lp) {
	// translate vertices to vertices in branching
	struct Vertex* v = branching->vertices[e->startPoint->number];
	struct Vertex* w = branching->vertices[e->endPoint->number];
	// find roots of components
	struct Vertex* rv = getRoot(v);
	struct Vertex* rw = getRoot(w);

	if (rv == rw) {
		return 0;
	} else {
		struct VertexList* f = getVertexList(lp);
		if (rv->d < rw->d) {
			f->startPoint = rv;
			f->endPoint = rw;
			addEdge(rv, f);
			if (rv->d == rw->d) {
				++rw->d;
			}
		} else {
			f->startPoint = rw;
			f->endPoint = rv;
			addEdge(rw, f);
			if (rv->d == rw->d) {
				++rv->d;
			}
		}
		return 1;
	}
}


struct ShallowGraph* kruskalMST(struct Graph* g, struct VertexList** sortedEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* branching = createGraph(g->n, gp);
	struct ShallowGraph* spanningTree = getShallowGraph(sgp);
	int i;

	for (i=0; i<g->m; ++i) {
		if (isCycleFree(branching, sortedEdges[i], sgp->listPool)) {
			appendEdge(spanningTree, shallowCopyEdge(sortedEdges[i], sgp->listPool));
		}
	}

	dumpGraph(gp, branching);
	return spanningTree;
}
