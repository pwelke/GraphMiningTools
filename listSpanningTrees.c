#include <stdlib.h>

#include "graph.h"
#include "dfs.h"

void visit(struct Vertex* v, int component, int* components) {
	struct VertexList* e;

	if (components[v->number] != -1) {
		return;
	}

	components[v->number] = component;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		visit(e->endPoint, component, components);
	}
}

struct ShallowGraph* selectB(struct Graph* partialTree, struct ShallowGraph* rest, int* components, int n, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* B = getShallowGraph(sgp);
	struct VertexList* e;
	int v;

	for (v=0; v<n; ++v) {
		components[v] = -1;
	}

	/* mark connected components */
	for (v=0; v<n; ++v) {
		visit(partialTree->vertices[v], v, components);
	}

	B->next = getShallowGraph(sgp);
	for (e=rest->edges; e!=NULL; e=e->next) {
		int v = e->startPoint->number;
		int w = e->endPoint->number;
		if ((components[v] != -1) && (components[w] != -1) && (components[v] == components[w])) {
			e->flag = 1;
		}
	}
	for (e=rest->edges; e!=NULL; e=rest->edges) {
		if (e->flag == 1) {
			pushEdge(B, popEdge(rest));
		} else {
			pushEdge(B->next, popEdge(rest));
		}
	}
	return B;
}


struct ShallowGraph* rec(struct Graph* graph, struct Graph* partialTree, struct ShallowGraph* rest, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct VertexList* e;
	struct ShallowGraph* B;

	if (graph->m == partialTree->m) {
		// debug
		printGraph(partialTree);
		return getGraphEdges(partialTree, sgp);
	}

	e = popEdge(rest);
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);

	/* S1 */
	/* let B be the set of all edges (in graph) joining edges already connected in partialTree */
	B = selectB(partialTree, rest, components, n, sgp);

	deleteEdges(graph, B, gp);

	rec(graph, partialTree, B->next, components, n, sgp, gp);

	addEdges(graph, B, gp);

	deleteEdgeBetweenVertices(graph, e, gp);
	deleteEdgeBetweenVertices(partialTree, e, gp);

	/* S2 */


	return NULL;
}

struct ShallowGraph* listSpanningTrees(struct Graph* original, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	
	struct Graph* graph = cloneGraph(original, gp);
	struct Graph* partialTree = emptyGraph(original, gp);
	struct ShallowGraph* rest = getShallowGraph(sgp);
	struct ShallowGraph* result = NULL;
	int* components = malloc(original->n * sizeof(int));

	struct ShallowGraph* idx;

	/* add all bridges to partialTree, all other to graph */
	struct ShallowGraph* biconnectedComponents = findBiconnectedComponents(original, sgp);
	for (idx=biconnectedComponents; idx; idx=idx->next) {
		struct VertexList* e = idx->edges;
		if (idx->m == 1) {
			addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);
		} else {
			while (idx->edges != NULL) {
				pushEdge(rest, popEdge(idx));
			}
		}
	}
	dumpShallowGraph(sgp, biconnectedComponents);

	/* do the backtrack, baby */
	result = rec(graph, partialTree, rest, components, original->n, sgp, gp);

	free(components);

	return result;
}

