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
#include "treeEnumerationRooted.h"


/** 
Take a tree and add any edge in the candidate set to each vertex in the graph in turn.
candidateEdges is expected to contain edges that have a nonNULL ->startPoint and ->endPoint.
The label of startPoint determines, which vertex can be used for appending the edge, 
the label of the endpoint defines the label of the new vertex added

TODO there is speedup to gain here. The appending at the moment runs in o(#candidates * n).
 */
struct Graph* extendRootedPatternAllWays(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp) {
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
struct IntSet* aprioriCheckExtensionRootedReturnList(struct Graph* extension, struct Vertex* lowerLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

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
			struct ShallowGraph* subString = canonicalStringOfRootedTree(subgraph->vertices[0], subgraph->vertices[0], sgp);
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
