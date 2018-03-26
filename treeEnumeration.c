#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "graph.h"
#include "cs_Tree.h"
#include "searchTree.h"
#include "bloomFilter.h"
#include "subtreeIsoUtils.h"
#include "outerplanar.h"
#include "intSet.h"
#include "treeCenter.h"
#include "treeEnumeration.h"

/**
 Creates a hard copy of g with g->n + 1 vertices and g->m + 1 edges that has a new vertex that is a
 copy of newEdge->endPoint and a new edge from the vertex v in g to the new vertex. with label newEdge->label.

 Does not check if newEdge->startPoint->label == g->vertices[v]->label!

 The newly added vertex v has v->number = g->n, the vertex numbers of the vertices of g do not change.
 */
struct Graph* refinementGraph(struct Graph* g, int v, struct VertexList* newEdge, struct GraphPool* gp) {
	struct Graph* copy = getGraph(gp);
	int i;

	copy->activity = g->activity;
	copy->m = g->m; // addEdgebetweenVertices adds plus 1 itself
	copy->n = g->n + 1;
	copy->number = g->number;

	copy->vertices = malloc(copy->n * sizeof(struct Vertex*));

	/* copy vertices */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			copy->vertices[i] = shallowCopyVertex(g->vertices[i], gp->vertexPool);
		}
	}

	/* copy new vertex and edge */
	copy->vertices[copy->n - 1] = shallowCopyVertex(newEdge->endPoint, gp->vertexPool);
	copy->vertices[copy->n - 1]->number = copy->n - 1;
	addEdgeBetweenVertices(v, copy->n - 1, newEdge->label, copy, gp);

	/* copy edges */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			struct VertexList* e;
			for (e=g->vertices[i]->neighborhood; e; e=e->next) {
				struct VertexList* tmp = getVertexList(gp->listPool);
				tmp->endPoint = copy->vertices[e->endPoint->number];
				tmp->startPoint = copy->vertices[e->startPoint->number];
				tmp->label = e->label;

				/* add the shallow copy to the new graph */
				tmp->next = copy->vertices[e->startPoint->number]->neighborhood;
				copy->vertices[e->startPoint->number]->neighborhood = tmp;
			}
		}
	}
	return copy;
}


/** 
Take a tree and add any edge in the candidate set to each vertex in the graph in turn.
candidateEdges is expected to contain edges that have a nonNULL ->startPoint and ->endPoint.
The label of startPoint determines, which vertex can be used for appending the edge, 
the label of the endpoint defines the label of the new vertex added

TODO there is speedup to gain here. The appending at the moment runs in o(#candidates * n).
 */
struct Graph* extendPatternAllWays(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp) {
	struct Graph* rho = NULL;
	struct VertexList* e;
	for (e=candidateEdges->edges; e!=NULL; e=e->next) {
		int v;
		for (v=0; v<g->n; ++v) {
			/* if the labels are compatible */
			if (strcmp(g->vertices[v]->label, e->startPoint->label) == 0) {
				struct Graph* h = refinementGraph(g, v, e, gp);
				h->next = rho;
				rho = h;
			}
		}
	}
	return rho;
}


/**
Take a tree and add any edge in the candidate set to the leaves and their neighbors.
This is a speedup compared to the extendPatternAllWays() method, as it is complete (i.e., all trees are
reachable from the empty tree by consecutive extension) but some 'inner' vertices are not extended on.

candidateEdges is expected to contain edges that have a nonNULL ->startPoint and ->endPoint.
The label of startPoint determines, which vertex can be used for appending the edge,
the label of the endpoint defines the label of the new vertex added

TODO there is speedup to gain here. The appending at the moment runs in o(#candidates * #leaves).
 */
struct Graph* extendPatternOnLeaves(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp) {

	struct Graph* rho = NULL;

	// init visited flags
	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->visited = 0;
	}

	// mark leaves and neighbors of leaves
	for (int v=0; v<g->n; ++v) {
		if (isLeaf(g->vertices[v])) {
			g->vertices[v]->visited = 1;
			g->vertices[v]->neighborhood->endPoint->visited = 1;
		}
	}

	// special case: singleton vertex should be extended, as well (even though it is not a leaf)
	if (g->n == 1) {
		g->vertices[0]->visited = 1;
	}

	// add refinement edges only at leaves and neighbors of leaves
	for (int v=0; v<g->n; ++v) {
		if (g->vertices[v]->visited) {
			for (struct VertexList* e=candidateEdges->edges; e!=NULL; e=e->next) {
				/* if the labels are compatible */
				if (strcmp(g->vertices[v]->label, e->startPoint->label) == 0) {
					struct Graph* h = refinementGraph(g, v, e, gp);
					h->next = rho;
					rho = h;
				}
			}
		}
	}
	return rho;
}




/**
Take a tree and add any edge in the candidate set to the vertices on the outermost two shells.
By a 'shell' we mean a set of vertices that have the same distance from the center of the pattern tree.
This is a speedup compared to the extendPatternOnLeaves() method, as it is complete (i.e., all trees are
reachable from the empty tree by consecutive extension) but some 'inner' leaves and their neighbors are
not extended on.

candidateEdges is expected to contain edges that have a nonNULL ->startPoint and ->endPoint.
The label of startPoint determines, which vertex can be used for appending the edge,
the label of the endpoint defines the label of the new vertex added

TODO there is speedup to gain here. The appending at the moment runs in o(#candidates * #vertices in outer two shells).
 */
struct Graph* extendPatternOnOuterShells(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct Graph* rho = NULL;

	// if v->lowPoint >= outershell then we extend on v
	int outerShell = computeDistanceToCenter(g, sgp) - 1;

	// add refinement edges only at outer shell vertices
	for (int v=0; v<g->n; ++v) {
		if (g->vertices[v]->lowPoint >= outerShell) {
			for (struct VertexList* e=candidateEdges->edges; e!=NULL; e=e->next) {
				/* if the labels are compatible */
				if (strcmp(g->vertices[v]->label, e->startPoint->label) == 0) {
					struct Graph* h = refinementGraph(g, v, e, gp);
					h->next = rho;
					rho = h;
				}
			}
		}
	}
	return rho;
}



/**
Return the list of all graphs in extension that are not contained in listOfGraphs (given as searchTree).
extension is consumed. after the call, canonical strings of all elements in extension will be added to listOfGraphs */
struct Graph* basicFilter(struct Graph* extension, struct Vertex* listOfGraphs, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* result = NULL;
	struct Graph* g = extension;
	while (g != NULL) {
		// store second element, detach head
		struct Graph* tmp = g->next;
		g->next = NULL;

		struct ShallowGraph* string = canonicalStringOfTree(g, sgp);
		char alreadyFound = containsString(listOfGraphs, string);
		if (!alreadyFound) {
			addToSearchTree(listOfGraphs, string, gp, sgp);
			// add to result
			g->next = result;
			result = g;
		} else {
			dumpShallowGraph(sgp, string);
			dumpGraph(gp, g);
		}
		// move on
		g = tmp;
	}
	return result;
}


/**
for a given extension graph g several tests are run:

- any subtree of g with n-1 vertices (called an apriori parent) must be contained in lowerLevel
  (this ensures the apriori property that all subtrees are frequent).

if g fulfills this conditions, the method returns a list of the ids of the apriori parents in resultSetStore.
if g does not, NULL is returned.

Differences to other extension filter methods in this module:
This method only processes a single extension graph at a time. It does not do anything to it.
In contrast to the above filterExtension, this method does not use any hashing.
In contrast to aprioriFilterExtension, this method also returns a list of IntSets, that contain the ids of
the relevant (n-1)-vertex subtrees and sets h->number to its id (obtained by getID(currentLevel, cString(h)).
 */
struct IntSet* aprioriCheckExtensionReturnList(struct Graph* extension, struct Vertex* lowerLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	// returning NULL will indicate that apriori property does not hold for current
	struct IntSet* aprioriTreesOfExtension = getIntSet();

	// create graph that will hold subgraphs of size n-1 (this assumes that all extension trees have the same size)
	struct Graph* subgraph = getGraph(gp);
	setVertexNumber(subgraph, extension->n - 1);
	subgraph->m = subgraph->n - 1;

	for (int v=0; v<extension->n; ++v) {
		// if the removed vertex is a leaf, we test if the resulting subtree is contained in the lower level
		if (isLeaf(extension->vertices[v]) == 1) {
			// we invalidate current by removing the edge to v from its neighbor, which makes subgraph a valid tree
			struct VertexList* edge = snatchEdge(extension->vertices[v]->neighborhood->endPoint, extension->vertices[v]);
			// now copy pointers of all vertices \neq v to subgraph, this results in a tree of size current->n - 1
			int j = 0;
			for (int i=0; i<extension->n; ++i) {
				if (i == v) {
					continue; // ...with next vertex vertices
				} else {
					subgraph->vertices[j] = extension->vertices[i];
					subgraph->vertices[j]->number = j;
					++j;
				}
			}

			// test apriori property
			struct ShallowGraph* subString = canonicalStringOfTree(subgraph, sgp);
			int aprioriTreeID = getID(lowerLevel, subString);
			dumpShallowGraph(sgp, subString);

			// restore law and order in current (and invalidate subgraph)
			addEdge(edge->startPoint, edge);

			if (aprioriTreeID == -1) {
				dumpIntSet(aprioriTreesOfExtension);
				aprioriTreesOfExtension = NULL;
				break; // looping through the vertices of current and continue with next refinement
			} else {
				addIntSortedNoDuplicates(aprioriTreesOfExtension, aprioriTreeID);
			}
		}
	}

	// clean up
	for (int v=0; v<extension->n; ++v) {
		extension->vertices[v]->number = v;
	}
	// garbage collection
	for (int v=0; v<subgraph->n; ++v) {
		subgraph->vertices[v] = NULL;
	}
	dumpGraph(gp, subgraph);

	if (aprioriTreesOfExtension != NULL) {
		assert(isSortedUniqueIntSet(aprioriTreesOfExtension));
		return aprioriTreesOfExtension;
	} else {
		return NULL;
	}
}
