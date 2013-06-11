#include <stdlib.h>
//debug
#include <stdio.h>

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

char existsEdge(struct Graph* g, int v, int w) {
	struct VertexList* e;
	for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->number == w) {
			return 1;
		}
	}
	return 0;
}

/**
Split list into two new lists. the returned list r contains all edges in list that have a corresponding edge in graph. r->next contains all
edges that are not contained in graph. list is consumed and dumped. Any access to the list lateron will result in strange things happening.
*/
struct ShallowGraph* getGraphAndNonGraphEdges(struct Graph* graph, struct ShallowGraph* list, struct ShallowGraphPool* sgp) {
	struct VertexList* e;
	struct ShallowGraph* graphEdges = getShallowGraph(sgp);
	struct ShallowGraph* nonGraphEdges = getShallowGraph(sgp);

	/* mark edges that are in graph */
	for (e=list->edges; e!=NULL; e=e->next) {
		e->flag = existsEdge(graph, e->startPoint->number, e->endPoint->number);
	}

	/* consume list putting differently flagged edges in different lists */
	for (e=list->edges; e!=NULL; e=list->edges) {
		if (e->flag == 1) {
			pushEdge(graphEdges, popEdge(list));
		} else {
			pushEdge(nonGraphEdges, popEdge(list));
		}
	}
	dumpShallowGraph(sgp, list);

	graphEdges->next = nonGraphEdges;
	nonGraphEdges->prev = graphEdges;
	return graphEdges;
}


struct ShallowGraph* getNonTreeBridges(struct Graph* graph, struct Graph* partialTree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* tmp = getShallowGraph(sgp);
	struct ShallowGraph* biconnectedComponents = findBiconnectedComponents(graph, sgp);
	struct ShallowGraph* idx;
	struct ShallowGraph* B = getShallowGraph(sgp);

	for (idx=biconnectedComponents; idx; idx=idx->next) {
		if (idx->m == 1) {
			pushEdge(tmp, popEdge(idx));
		} 
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);

	tmp = getGraphAndNonGraphEdges(partialTree, tmp, sgp);
	B = tmp->next;
	tmp->next = NULL;
	dumpShallowGraph(sgp, tmp);

	return B;
}


struct ShallowGraph* rec(struct Graph* graph, struct Graph* partialTree, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct VertexList* e;
	struct ShallowGraph* result1;
	struct ShallowGraph* result2;
	struct ShallowGraph* B;
	struct ShallowGraph* bridges;
	struct ShallowGraph* rest = getGraphAndNonGraphEdges(partialTree, getGraphEdges(graph, sgp), sgp);
	rest = rest->next;

	if (graph->m == partialTree->m) {
		struct ShallowGraph* tree = getGraphEdges(partialTree, sgp);
		tree->next = tree->prev = tree;
		// //debug
		// printf("Spanning tree has %i edges\n", tree->m);
		return tree;
	}

	e = popEdge(rest);
	//debug
	printf("popping ");
	printVertexList(e);

	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);

	/* S1 */
	/* let B be the set of all edges (in graph) joining edges already connected in partialTree */
	B = selectB(partialTree, rest, components, n, sgp);

	deleteEdges(graph, B, gp);

	result1 = rec(graph, partialTree, components, n, sgp, gp);
	//debug 
	dumpShallowGraphCycle(sgp, result1);
	result1 = NULL;

	addEdges(graph, B, gp);

	deleteEdgeBetweenVertices(graph, e, gp);
	deleteEdgeBetweenVertices(partialTree, e, gp);

	/* S2 */
	bridges = getNonTreeBridges(graph, partialTree, sgp);
	addEdges(partialTree, bridges, gp);
	result2 = rec(graph, partialTree, components, n, sgp, gp);
	//debug 
	dumpShallowGraphCycle(sgp, result2);
	result2 = NULL;

	deleteEdges(partialTree, bridges, gp);
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, graph, gp);

	/* garbage collection */
	dumpShallowGraphCycle(sgp, B);
	dumpShallowGraphCycle(sgp, bridges);
	dumpShallowGraphCycle(sgp, rest->prev);

	// at this point, spanningTree and graph need to be same as above.

	return addComponent(result1, result2);
}

struct ShallowGraph* listSpanningTrees(struct Graph* original, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	
	struct Graph* graph = cloneGraph(original, gp);
	struct Graph* partialTree = emptyGraph(original, gp);
	struct ShallowGraph* result = NULL;
	int* components = malloc(sizeof(int) * original->n);
	struct ShallowGraph* idx;

	/* add all bridges to partialTree */
	struct ShallowGraph* biconnectedComponents = findBiconnectedComponents(original, sgp);
	for (idx=biconnectedComponents; idx; idx=idx->next) {
		struct VertexList* e = idx->edges;
		if (idx->m == 1) {
			addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);
		}
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	

	//debug
	printf("Graph %i has %i vertices and %i edges\n", original->number, original->n, original->m);

	/* do the backtrack, baby */
	result = rec(graph, partialTree, components, graph->n, sgp, gp);

	/* garbage collection */
	free(components);
	dumpGraph(gp, graph);
	dumpGraph(gp, partialTree);

	return result;
}
