#include <stdlib.h>
#include <limits.h>
#include <malloc.h>

#include "searchTree.h"
#include "cs_Tree.h"
#include "listComponents.h"
#include "listSpanningTrees.h"


/** Implements Read&Tarjans listing algorithm for spanning 
trees of a graph. 

Two variants are given. A listing algorithm that returns a
list of all spanning trees, and a version that counts all spanning
trees. The second method works, if the number of spanning trees is 
too large to be materialized at once in main memory. However, 
the runtime of the algorithm still is O(n * nSpanningTrees).
There is a parameter for stopping the computation, if the number 
of trees already found superceeds a threshold. */


void __visit(struct Vertex* v, int component, int* components) {
	struct VertexList* e;

	if (components[v->number] != -1) {
		return;
	}

	components[v->number] = component;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		__visit(e->endPoint, component, components);
	}
}


struct ShallowGraph* __selectB(struct Graph* partialTree, struct ShallowGraph* rest, int* components, int n, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* B = getShallowGraph(sgp);
	struct VertexList* e;
	int v;

	for (v=0; v<n; ++v) {
		components[v] = -1;
	}

	/* mark connected components */
	for (v=0; v<n; ++v) {
		__visit(partialTree->vertices[v], v, components);
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
struct ShallowGraph* __getGraphAndNonGraphEdges(struct Graph* graph, struct ShallowGraph* list, struct ShallowGraphPool* sgp) {
	struct VertexList* e;
	struct ShallowGraph* graphEdges = getShallowGraph(sgp);
	struct ShallowGraph* nonGraphEdges = getShallowGraph(sgp);

	/* mark edges that are in graph */
	for (e=list->edges; e!=NULL; e=e->next) {
		e->flag = isNeighbor(graph, e->startPoint->number, e->endPoint->number);
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


struct ShallowGraph* __getNonTreeBridges(struct Graph* graph, struct Graph* partialTree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* tmp = getShallowGraph(sgp);
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(graph, sgp);
	struct ShallowGraph* idx;
	struct ShallowGraph* nonTreeBridges;

	/* find bridges in graph */
	for (idx=biconnectedComponents; idx; idx=idx->next) {
		if (idx->m == 1) {
			pushEdge(tmp, popEdge(idx));
		} 
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);

	tmp = __getGraphAndNonGraphEdges(partialTree, tmp, sgp);
	nonTreeBridges = tmp->next;
	tmp->next = NULL;
	dumpShallowGraph(sgp, tmp);

	return nonTreeBridges;
}


struct ShallowGraph* __rec(int d, struct Graph* graph, struct Graph* partialTree, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {

	struct VertexList* e;
	struct ShallowGraph* result1;
	struct ShallowGraph* result2;
	struct ShallowGraph* B;
	struct ShallowGraph* bridges;
	struct ShallowGraph* graphEdges = __getGraphAndNonGraphEdges(partialTree, getGraphEdges(graph, sgp), sgp);
	struct ShallowGraph* rest = graphEdges->next;

	if (rest->m == 0) {
		struct ShallowGraph* tree = getGraphEdges(partialTree, sgp);
		tree->next = tree->prev = tree;

		/* garbage collection */
		dumpShallowGraphCycle(sgp, graphEdges);

		return tree;
	}
	/* get edge that will be included and excluded in this __recursive step */
	e = popEdge(rest);

	/* add e to partialTree (already contained in graph) to find spanning trees containing e */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);

	/* S1 */
	/* let B be the set of all edges (in graph) joining edges already connected in partialTree */
	B = __selectB(partialTree, rest, components, n, sgp);

	/* ensures, that adding an edge of graph to partialTree results in a tree in the __recursive call */
	deleteEdges(graph, B, gp);

	result1 = __rec(d+1, graph, partialTree, components, n, sgp, gp);

	/* add edges to graph again */
	addEdges(graph, B, gp);

	/* preliminary garbage collection */
	dumpShallowGraphCycle(sgp, B);

	/* delete e from partialTree and graph to find spanning trees not containing e */
	deleteEdgeBetweenVertices(graph, e, gp);
	deleteEdgeBetweenVertices(partialTree, e, gp);

	/* S2 */
	bridges = __getNonTreeBridges(graph, partialTree, sgp);

	addEdges(partialTree, bridges, gp);
	result2 = __rec(d+1, graph, partialTree, components, n, sgp, gp);

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
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(original, sgp);
	for (idx=biconnectedComponents; idx; idx=idx->next) {
		struct VertexList* e = idx->edges;
		if (idx->m == 1) {
			addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);
		}
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	
	/* do the backtrack, baby */
	result = __rec(0, graph, partialTree, components, graph->n, sgp, gp);
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



/** TODO: nasty! merge with above */
struct ShallowGraph* __recK(int d, int* k, struct Graph* graph, struct Graph* partialTree, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {

	struct VertexList* e;
	struct ShallowGraph* result1;
	struct ShallowGraph* result2;
	struct ShallowGraph* B;
	struct ShallowGraph* bridges;
	struct ShallowGraph* graphEdges = __getGraphAndNonGraphEdges(partialTree, getGraphEdges(graph, sgp), sgp);
	struct ShallowGraph* rest = graphEdges->next;

	if (rest->m == 0) {
		struct ShallowGraph* tree = getGraphEdges(partialTree, sgp);
		tree->next = tree->prev = tree;

		/* garbage collection */
		dumpShallowGraphCycle(sgp, graphEdges);

		/* count this spanning tree */
		*k -= 1;

		return tree;
	}
	/* get edge that will be included and excluded in this recursive step */
	e = popEdge(rest);

	/* add e to partialTree (already contained in graph) to find spanning trees containing e */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);

	/* S1 */
	/* let B be the set of all edges (in graph) joining edges already connected in partialTree */
	B = __selectB(partialTree, rest, components, n, sgp);

	/* ensures, that adding an edge of graph to partialTree results in a tree in the recursive call */
	deleteEdges(graph, B, gp);

	result1 = __recK(d+1, k, graph, partialTree, components, n, sgp, gp);

	/* add edges to graph again */
	addEdges(graph, B, gp);

	/* preliminary garbage collection */
	dumpShallowGraphCycle(sgp, B);

	/* if we did find enough spanning trees, skip second part */
	if (*k <= 0) {
		return result1;
	}

	/* delete e from partialTree and graph to find spanning trees not containing e */
	deleteEdgeBetweenVertices(graph, e, gp);
	deleteEdgeBetweenVertices(partialTree, e, gp);

	/* S2 */
	bridges = __getNonTreeBridges(graph, partialTree, sgp);

	addEdges(partialTree, bridges, gp);
	result2 = __recK(d+1, k, graph, partialTree, components, n, sgp, gp);

	deleteEdges(partialTree, bridges, gp);

	/* At the end, spanningTree and graph need to be same as in the beginning. */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, graph, gp);

	/* garbage collection */
	dumpVertexList(sgp->listPool, e);
	dumpShallowGraphCycle(sgp, bridges);
	dumpShallowGraphCycle(sgp, graphEdges);

	return addComponent(result1, result2);
}


/** return the first k spanning trees of original as a CYCLE OF SHALLOW GRAPHS ! */
struct ShallowGraph* listKSpanningTrees(struct Graph* original, int* k, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	
	struct Graph* graph = cloneGraph(original, gp);
	struct Graph* partialTree = emptyGraph(original, gp);
	struct ShallowGraph* result = NULL;
	int* components = malloc(sizeof(int) * original->n);
	struct ShallowGraph* idx;

	/* add all bridges to partialTree */
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(original, sgp);
	for (idx=biconnectedComponents; idx; idx=idx->next) {
		struct VertexList* e = idx->edges;
		if (idx->m == 1) {
			addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);
		}
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	
	/* do the backtrack, baby */
	result = __recK(0, k, graph, partialTree, components, graph->n, sgp, gp);
	// do not disconnect the cycle for easy access to last element
	// result->prev->next = NULL;
	// result->prev = NULL;

	/* edges in result point to vertices in partialTree this has to be changed */
	rebaseShallowGraph(result, original);

	/* garbage collection */
	free(components);
	dumpGraph(gp, graph);
	dumpGraph(gp, partialTree);

	return result;
}


long int __recCount(long int d, struct Graph* graph, struct Graph* partialTree, int* components, int n, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* B;

	struct ShallowGraph* graphEdges = __getGraphAndNonGraphEdges(partialTree, getGraphEdges(graph, sgp), sgp);
	struct ShallowGraph* rest = graphEdges->next;

	if (rest->m == 0) {
		/* garbage collection */
		dumpShallowGraphCycle(sgp, graphEdges);
		return 1;
	}

	/* get edge that will be included and excluded in this __recursive step */
	struct VertexList* e = popEdge(rest);

	/* add e to partialTree (already contained in graph) to find spanning trees containing e */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);

	/* S1 */
	/* let B be the set of all edges (in graph) joining edges already connected in partialTree */
	B = __selectB(partialTree, rest, components, n, sgp);

	/* ensures, that adding an edge of graph to partialTree results in a tree in the __recursive call */
	deleteEdges(graph, B, gp);

	long int result1 = __recCount(d, graph, partialTree, components, n, sgp, gp);

	/* add edges to graph again */
	addEdges(graph, B, gp);

	/* preliminary garbage collection */
	dumpShallowGraphCycle(sgp, B);

	/* at this point, graph and partialTree are as at the start of this call.
	If there are already too many spanning Trees found (indicated by return value -1)
	return at this point and prune the second __recursive call */
	if ((result1 == -1) || (result1 > d)) {
		/* garbage collection */
		dumpVertexList(sgp->listPool, e);
		dumpShallowGraphCycle(sgp, graphEdges);
		return -1;
	}

	/* delete e from partialTree and graph to find spanning trees not containing e */
	deleteEdgeBetweenVertices(graph, e, gp);
	deleteEdgeBetweenVertices(partialTree, e, gp);

	/* S2 */
	struct ShallowGraph* bridges = __getNonTreeBridges(graph, partialTree, sgp);

	addEdges(partialTree, bridges, gp);
	long int result2 = __recCount(d, graph, partialTree, components, n, sgp, gp);
	deleteEdges(partialTree, bridges, gp);

	/* At the end, spanningTree and graph need to be same as in the beginning. */
	addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, graph, gp);

	/* garbage collection */
	dumpVertexList(sgp->listPool, e);
	dumpShallowGraphCycle(sgp, bridges);
	dumpShallowGraphCycle(sgp, graphEdges);

	if ((result2 == -1) || (result2 > d)) {
		/* if there were too many spanning trees in the second __recursion... */ 
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


long int countSpanningTrees(struct Graph* original, long int maxBound, struct ShallowGraphPool* sgp, struct GraphPool* gp) {

	struct Graph* graph = cloneGraph(original, gp);
	struct Graph* partialTree = emptyGraph(original, gp);
	long int result;
	int* components = malloc(sizeof(int) * original->n);
	struct ShallowGraph* idx;

	/* add all bridges to partialTree */
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(original, sgp);
	for (idx=biconnectedComponents; idx; idx=idx->next) {
		struct VertexList* e = idx->edges;
		if (idx->m == 1) {
			addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, partialTree, gp);
		}
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	
	/* do the backtrack, baby */
	result = __recCount(maxBound, graph, partialTree, components, graph->n, sgp, gp);

	/* garbage collection */
	free(components);
	dumpGraph(gp, graph);
	dumpGraph(gp, partialTree);

	return result;
}


int countNonisomorphicSpanningTrees(struct Graph* g, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int i = 0;

	struct Vertex* searchTree = getVertex(gp->vertexPool);

	// list the spanning trees, canonicalize them and add them in a search tree (to avoid duplicates, i.e. isomorphic spanning trees
	struct ShallowGraph* sample = listSpanningTrees(g, sgp, gp);
	for (struct ShallowGraph* tree=sample; tree!=NULL; tree=tree->next) {
		if (tree->m != 0) {
			struct Graph* tmp = shallowGraphToGraph(tree, gp);
			struct ShallowGraph* cString = canonicalStringOfTree(tmp, sgp);
			addToSearchTree(searchTree, cString, gp, sgp);
			/* garbage collection */
			dumpGraph(gp, tmp);
		}
	}

	// this is the number of isomorphism classes
	i = searchTree->d;

	dumpShallowGraphCycle(sgp, sample);
	dumpSearchTree(gp, searchTree);

	return i;
}
