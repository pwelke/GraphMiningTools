#include <stdlib.h>
#include "graph.h"
#include "sampling.h"


struct ShallowGraph* sampleSpanningTree(struct Graph* g, struct ShallowGraphPool* sgp) {
	int dbg_countIterations = 0;
	int v;
	int root;
	int verticesLeft = g->n - 1;
	struct ShallowGraph* spanningTree;

	if ((g == NULL) || (g->n == 0)) {
		return NULL;
	}

	for (v=0; v<g->n; ++v) {
		g->vertices[v]->visited = 0;
	}
	spanningTree = getShallowGraph(sgp);

	/* use some random start vertex */
	root = rand() % g->n;
	g->vertices[root]->visited = 1;

	while (verticesLeft > 0) {
		++dbg_countIterations;

		struct VertexList* e;
		int next = rand() % degree(g->vertices[root]) + 1;
		/* go to the edge that was selected */
		for (v=0, e=g->vertices[root]->neighborhood; v<next; e=e->next, ++v);
		if (e->endPoint->visited != 0) {
			appendEdge(spanningTree, shallowCopyEdge(e, sgp->listPool));
			e->endPoint->visited = 1;
		}
		root = e->endPoint;
	}
	printf("%i\n", dbg_countIterations);
	return spanningTree;
}


struct VertexList* randomSuccessor(struct Vertex* root) {
	struct VertexList* e;
	int v;
	int next = rand() % degree(root);
	/* go to the edge that was selected */
	for (v=0, e=root->neighborhood; v<next; e=e->next, ++v);
	return e;
}

/** David Bruce Wilson:
Generating Random Spanning Trees More Quickly than the Cover Time.
Proceedings of the Twenty-eighth Annual ACM Symposium on the Theory of Computing 
(Philadelphia, PA, 1996), 296-303, ACM, New York, 1996 */
struct VertexList** sampleSpanningEdges(struct Graph* g, int root) {
	int dbg_countIterations = 0;
	int i, u;
	int verticesLeft = g->n - 1;
	struct VertexList** spEdges = malloc(g->n * sizeof(struct VertexList*));

	for (i=0; i<g->n; ++i) {
		g->vertices[i]->visited = 0;
		spEdges[i] = NULL;
	}

	g->vertices[root]->visited = 1;

	for (i=0; i<g->n; ++i) {
		 u = i;
		 while (g->vertices[u]->visited == 0) {
		 	spEdges[u] = randomSuccessor(g->vertices[u]);
		 	u = spEdges[u]->endPoint->number;
		 }
		 u = i;
		 while (g->vertices[u]->visited == 0) {
		 	g->vertices[u]->visited = 1;
		 	u = spEdges[u]->endPoint->number;
		 }
	}
	return spEdges;
}

struct ShallowGraph* sampleSpanningTree(struct Graph* g, int root, struct ShallowGraphPool* sgp) {
	struct VertexList** spanningEdges = sampleSpanningEdges(g, root);
	struct ShallowGraph* spanningTree = getShallowGraph(sgp);
	int v;
	for (v=0; v<g->n; ++v) {
		if (spanningEdges[i] != NULL) {
			appendEdge(spanningTree, shallowCopyEdge(spanningEdges[i], spg->listPool));
		}
	}
	free(spanningEdges);
	return spanningTree;
} 
