#include <stdlib.h>
#include <limits.h>
//debug
#include <stdio.h>
#include <string.h>

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

	for (e=rest->edges; e!=NULL; e=e->next) {
		int v = e->startPoint->number;
		int w = e->endPoint->number;
		if ((components[v] != -1) && (components[w] != -1) && (components[v] == components[w])) {
			e->flag = 1;
		}
	}

	B->next = getShallowGraph(sgp);
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
	if ((g->vertices[v]) && (g->vertices[v]->neighborhood)) {
		for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->number == w) {
				return 1;
			}
		}
	}
	/* if there is no vertex, or no neighborhood at all, or w does not occur
	as endpoint of any edge incident to v, return false */
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

/**
 * Print information about the edges contained in a graph to the screen
 */
void printGraphEdgesOfTwoGraphs(char* name, struct Graph *g, struct Graph* h) {
	int i;
	printf("%s 1 has %i vertices and %i edges:\n", name, g->n, g->m);
	printf("%s 2 has %i vertices and %i edges:\n", name, h->n, h->m);
	for (i=0; i<g->n; ++i) {
		printf("%i: ", i);
		if (g->vertices[i]) {
			struct VertexList *idx;
			for (idx = g->vertices[i]->neighborhood; idx; idx = idx->next) {
				printf("%i ", idx->endPoint->number);	
			}
		}
		printf("\n%i: ", i);
		if (h->vertices[i]) {
			struct VertexList *idx;
			for (idx = h->vertices[i]->neighborhood; idx; idx = idx->next) {
				printf("%i ", idx->endPoint->number);	
			}
		}
		printf("\n\n");
	}
}

struct ShallowGraph* getNonTreeBridges(struct Graph* graph, struct Graph* partialTree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* tmp = getShallowGraph(sgp);
	struct ShallowGraph* biconnectedComponents = findBiconnectedComponents(graph, sgp);
	struct ShallowGraph* idx;
	struct ShallowGraph* nonTreeBridges;

	/* find bridges in graph */
	for (idx=biconnectedComponents; idx; idx=idx->next) {
		if (idx->m == 1) {
			pushEdge(tmp, popEdge(idx));
		} 
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);

	tmp = getGraphAndNonGraphEdges(partialTree, tmp, sgp);
	nonTreeBridges = tmp->next;
	tmp->next = NULL;
	dumpShallowGraph(sgp, tmp);

	return nonTreeBridges;
}

void cleanString(char* s, int n) {
	int i;
	for (i=0; i<n; ++i) {
		s[i] = 0;
	}
}

char diffGraphs(char* name, struct Graph *g, struct Graph* h) {
	int n = 300;
	int diffs = 0;
	char* s1 = malloc(n * sizeof(char));
	char* s2 = malloc(n * sizeof(char));
	int i;
	cleanString(s1,n);
	cleanString(s2,n);

	sprintf(s1, "%s has %i vertices and %i edges:\n", name, g->n, g->m);
	sprintf(s2, "%s has %i vertices and %i edges:\n", name, h->n, h->m);

	if (strcmp(s1, s2) != 0) {
		++diffs;
	}
	for (i=0; i<g->n; ++i) {
		int store1 = 0;
		int store2 = 0;
		cleanString(s1, n);
		cleanString(s2, n);
		store1 += sprintf(s1, "%i: ", i);
		if (g->vertices[i]) {
			struct VertexList *idx;
			for (idx = g->vertices[i]->neighborhood; idx; idx = idx->next) {
				store1 += sprintf(s1+store1, "%i ", idx->endPoint->number);	
			}
		}
			store2 += sprintf(s2, "%i: ", i);
		if (h->vertices[i]) {
			struct VertexList *idx;
			for (idx = h->vertices[i]->neighborhood; idx; idx = idx->next) {
				store2 += sprintf(s2+store2, "%i ", idx->endPoint->number);	
			}
		}
		if (strcmp(s1, s2) != 0) {
			printf(s1);
			printf("\n");
			printf(s2);
			printf("\n\n");
			++diffs;
		}
	}
	free(s1);
	free(s2);

	if (diffs > 0) {
		printf("%s has %i vertices and %i edges:\n", name, g->n, g->m);
		printf("%s has %i vertices and %i edges:\n", name, h->n, h->m);
	}
	return diffs;
}


struct ShallowGraph* rec(int d, struct Graph* graph, struct Graph* partialTree, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {

	struct VertexList* e;
	struct ShallowGraph* result1;
	struct ShallowGraph* result2;
	struct ShallowGraph* B;
	struct ShallowGraph* bridges;
	struct ShallowGraph* graphEdges = getGraphAndNonGraphEdges(partialTree, getGraphEdges(graph, sgp), sgp);
	struct ShallowGraph* rest = graphEdges->next;

	//printShallowGraph(graphEdges);

	if (rest->m == 0) {
		struct ShallowGraph* tree = getGraphEdges(partialTree, sgp);
		tree->next = tree->prev = tree;

		// //debug
		// printf("Output has %i edges\n", tree->m);

		/* garbage collection */
		dumpShallowGraphCycle(sgp, graphEdges);

		return tree;
	}
	/* get edge that will be included and excluded in this recursive step */
	e = popEdge(rest);

	// //debug
	// printf("popping ");
	// printVertexList(e);
	// if (existsEdge(partialTree, e->startPoint->number, e->endPoint->number)) {
	// 	printf("e = (%i, %i) should not be contained in partialTree\n", e->startPoint->number, e->endPoint->number);
	// }

	/* add e to partialTree (already contained in graph) to find spanning trees containing e */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);

	/* S1 */
	/* let B be the set of all edges (in graph) joining edges already connected in partialTree */
	B = selectB(partialTree, rest, components, n, sgp);

	/* ensures, that adding an edge of graph to partialTree results in a tree in the recursive call */
	deleteEdges(graph, B, gp);

	result1 = rec(d+1, graph, partialTree, components, n, sgp, gp);
	// //debug 
	// dumpShallowGraphCycle(sgp, result1);
	// result1 = NULL;

	// //debug
	// printGraphEdges(partialTree);
	// printShallowGraph(B);
	// return;

	/* add edges to graph again */
	addEdges(graph, B, gp);

	/* preliminary garbage collection */
	dumpShallowGraphCycle(sgp, B);

	/* delete e from partialTree and graph to find spanning trees not containing e */
	deleteEdgeBetweenVertices(graph, e, gp);
	deleteEdgeBetweenVertices(partialTree, e, gp);

	/* S2 */
	bridges = getNonTreeBridges(graph, partialTree, sgp);

	addEdges(partialTree, bridges, gp);
	result2 = rec(d+1, graph, partialTree, components, n, sgp, gp);
	// //debug 
	// dumpShallowGraphCycle(sgp, result2);
	// result2 = NULL;

	deleteEdges(partialTree, bridges, gp);

	/* At the end, spanningTree and graph need to be same as in the beginning. */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, graph, gp);

	/* garbage collection */
	dumpVertexList(sgp->listPool, e);
	dumpShallowGraphCycle(sgp, bridges);
	dumpShallowGraphCycle(sgp, graphEdges);

	return addComponent(result1, result2);
	// return NULL;
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
	

	// //debug
	// printf("Graph %i has %i vertices and %i edges\n", original->number, original->n, original->m);

	/* do the backtrack, baby */
	result = rec(0, graph, partialTree, components, graph->n, sgp, gp);
	result->prev->next = NULL;
	result->prev = NULL;

	/* garbage collection */
	free(components);
	dumpGraph(gp, graph);
	dumpGraph(gp, partialTree);

	return result;
}


long int recCount(int d, struct Graph* graph, struct Graph* partialTree, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct VertexList* e;
	struct ShallowGraph* B;
	struct ShallowGraph* bridges;
	struct ShallowGraph* graphEdges = getGraphAndNonGraphEdges(partialTree, getGraphEdges(graph, sgp), sgp);
	struct ShallowGraph* rest = graphEdges->next;
	long int result1;
	long int result2;

	if (rest->m == 0) {
		/* garbage collection */
		dumpShallowGraphCycle(sgp, graphEdges);
		return 1;
	}

	/* get edge that will be included and excluded in this recursive step */
	e = popEdge(rest);

	/* add e to partialTree (already contained in graph) to find spanning trees containing e */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);

	/* S1 */
	/* let B be the set of all edges (in graph) joining edges already connected in partialTree */
	B = selectB(partialTree, rest, components, n, sgp);

	/* ensures, that adding an edge of graph to partialTree results in a tree in the recursive call */
	deleteEdges(graph, B, gp);

	result1 = recCount(d+1, graph, partialTree, components, n, sgp, gp);

	/* add edges to graph again */
	addEdges(graph, B, gp);

	/* preliminary garbage collection */
	dumpShallowGraphCycle(sgp, B);

	/* delete e from partialTree and graph to find spanning trees not containing e */
	deleteEdgeBetweenVertices(graph, e, gp);
	deleteEdgeBetweenVertices(partialTree, e, gp);

	/* S2 */
	bridges = getNonTreeBridges(graph, partialTree, sgp);

	addEdges(partialTree, bridges, gp);
	result2 = recCount(d+1, graph, partialTree, components, n, sgp, gp);
	deleteEdges(partialTree, bridges, gp);

	/* At the end, spanningTree and graph need to be same as in the beginning. */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, graph, gp);

	/* garbage collection */
	dumpVertexList(sgp->listPool, e);
	dumpShallowGraphCycle(sgp, bridges);
	dumpShallowGraphCycle(sgp, graphEdges);

	if ((result1 > LONG_MAX / 2) || (result2 > LONG_MAX / 2)) {
		return -1;
	} else {
		return result1 + result2;
	}
}

long int countSpanningTrees(struct Graph* original, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	
	struct Graph* graph = cloneGraph(original, gp);
	struct Graph* partialTree = emptyGraph(original, gp);
	long int result;
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
	

	// //debug
	// printf("Graph %i has %i vertices and %i edges\n", original->number, original->n, original->m);

	/* do the backtrack, baby */
	result = recCount(0, graph, partialTree, components, graph->n, sgp, gp);

	/* garbage collection */
	free(components);
	dumpGraph(gp, graph);
	dumpGraph(gp, partialTree);

	return result;
}