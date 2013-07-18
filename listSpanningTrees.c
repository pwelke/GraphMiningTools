#include <stdlib.h>
#include <limits.h>
#include <malloc.h>

#include "graph.h"
#include "dfs.h"
#include "listSpanningTrees.h"

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


struct ShallowGraph* rec(int d, struct Graph* graph, struct Graph* partialTree, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {

	struct VertexList* e;
	struct ShallowGraph* result1;
	struct ShallowGraph* result2;
	struct ShallowGraph* B;
	struct ShallowGraph* bridges;
	struct ShallowGraph* graphEdges = getGraphAndNonGraphEdges(partialTree, getGraphEdges(graph, sgp), sgp);
	struct ShallowGraph* rest = graphEdges->next;

	if (rest->m == 0) {
		struct ShallowGraph* tree = getGraphEdges(partialTree, sgp);
		tree->next = tree->prev = tree;

		/* garbage collection */
		dumpShallowGraphCycle(sgp, graphEdges);

		return tree;
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

	result1 = rec(d+1, graph, partialTree, components, n, sgp, gp);

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

	deleteEdges(partialTree, bridges, gp);

	/* At the end, spanningTree and graph need to be same as in the beginning. */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, graph, gp);

	/* garbage collection */
	dumpVertexList(sgp->listPool, e);
	dumpShallowGraphCycle(sgp, bridges);
	dumpShallowGraphCycle(sgp, graphEdges);

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
	
	/* do the backtrack, baby */
	result = rec(0, graph, partialTree, components, graph->n, sgp, gp);
	result->prev->next = NULL;
	result->prev = NULL;

	/* edges in result point to vertices in partialTree this has to be changed */
	rebaseShallowGraph(result, original);

	/* garbage collection */
	free(components);
	dumpGraph(gp, graph);
	dumpGraph(gp, partialTree);

	return result;
}


long int recCount(long int d, struct Graph* graph, struct Graph* partialTree, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
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

	result1 = recCount(d, graph, partialTree, components, n, sgp, gp);

	/* add edges to graph again */
	addEdges(graph, B, gp);

	/* preliminary garbage collection */
	dumpShallowGraphCycle(sgp, B);

	/* at this point, graph and partialTree are as at the start of this call.
	If there are already too many spanning Trees found (indicated by return value -1)
	return at this point and prune the second recursive call */
	if (result1 == -1) {
		/* garbage collection */
		dumpVertexList(sgp->listPool, e);
		dumpShallowGraphCycle(sgp, graphEdges);
		return -1;
	}

	/* delete e from partialTree and graph to find spanning trees not containing e */
	deleteEdgeBetweenVertices(graph, e, gp);
	deleteEdgeBetweenVertices(partialTree, e, gp);

	/* S2 */
	bridges = getNonTreeBridges(graph, partialTree, sgp);

	addEdges(partialTree, bridges, gp);
	result2 = recCount(d, graph, partialTree, components, n, sgp, gp);
	deleteEdges(partialTree, bridges, gp);

	/* At the end, spanningTree and graph need to be same as in the beginning. */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, graph, gp);

	/* garbage collection */
	dumpVertexList(sgp->listPool, e);
	dumpShallowGraphCycle(sgp, bridges);
	dumpShallowGraphCycle(sgp, graphEdges);

	if (result2 == -1) {
		/* if there were too many spanning trees in the second recursion... */ 
		return -1;
	} else {
		/* user set threshold on number of spanning trees */
		if ((result1 > d) || (result2 > d)) {
			return -1;
		} else {
			/* avoid overflow */
			if ((result1 > LONG_MAX / 2) || (result2 > LONG_MAX / 2)) {
				return -1;
			} else {
				return result1 + result2;
			}
		}
	}
}

long int countSpanningTrees(struct Graph* original, long int maxBound, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	
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
	
	/* do the backtrack, baby */
	result = recCount(maxBound, graph, partialTree, components, graph->n, sgp, gp);

	/* garbage collection */
	free(components);
	dumpGraph(gp, graph);
	dumpGraph(gp, partialTree);

	return result;
}