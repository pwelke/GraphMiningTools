#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "newCube.h"
#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsoUtils.h"
#include "bitSet.h"
#include "cachedGraph.h"
#include "iterativeSubtreeIsomorphism.h"


// MISC TOOLING

/* vertices of g have their ->visited values set to the postorder. Thus,
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
static struct Graph* makeBipartiteInstanceFromVertices(struct SubtreeIsoDataStore data, struct Vertex* removalVertex, struct Vertex* u, struct Vertex* v, struct GraphPool* gp) {
	struct Graph* B;

	int sizeofX = degree(u);
	int sizeofY = degree(v);

	// if one of the neighbors of u is removed from this instance to see if there is a matching covering all neighbors but it,
	// the bipartite graph must be smaller
	if (removalVertex->number != u->number) {
		--sizeofX;
	}

	/* construct bipartite graph B(v,u) */
	B = createGraph(sizeofX + sizeofY, gp);
	/* store size of first partitioning set */
	B->number = sizeofX;

	/* add vertex numbers of original vertices to ->lowPoint of each vertex in B
	and add edge labels to vertex labels to compare edges easily */
	int i = 0;
	for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->number != removalVertex->number) {
			B->vertices[i]->lowPoint = e->endPoint->number;
			B->vertices[i]->label = e->label;
			++i;
		}
	}
	for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
		B->vertices[i]->lowPoint = e->endPoint->number;
		B->vertices[i]->label = e->label;
		++i;
	}

	/* add edge (x,y) if u in S(y,x) */
	for (i=0; i<sizeofX; ++i) {
		int x = B->vertices[i]->lowPoint;
		for (int j=sizeofX; j<B->n; ++j) {
			int y = B->vertices[j]->lowPoint;

			/* y has to be a child of v */
			if (data.g->vertices[y]->visited < v->visited) {
				/* edge labels have to match, (v, child)->label in g == (u, child)->label in h
				these values were stored in B->vertices[i,j]->label */
				if (labelCmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
					if (containsCharacteristic(data, u, data.h->vertices[x], data.g->vertices[y])) {
						addResidualEdges(B->vertices[i], B->vertices[j], gp->listPool);
						++B->m;
					}
				}
			}
		}
	}

	return B;
}


int computeCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v, struct GraphPool* gp) {
	// TODO speedup by handling leaf case separately
	struct Graph* B = makeBipartiteInstanceFromVertices(data, y, u, v, gp);
	int sizeofMatching = bipartiteMatchingEvenMoreDirty(B);

	int nNeighbors = B->number;
	dumpGraph(gp, B);
	return (sizeofMatching == nNeighbors) ? 1 : 0;
}

/* vertices of g have their ->visited values set to the postorder. Thus,
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
static struct Graph* makeBipartiteInstanceFromVerticesCached(struct SubtreeIsoDataStore data, struct CachedGraph* cachedB, struct Vertex* removalVertex, struct Vertex* u, struct Vertex* v, struct GraphPool* gp) {

	int sizeofX = degree(u);
	int sizeofY = degree(v);

	// if one of the neighbors of u is removed from this instance to see if there is a matching covering all neighbors but it,
	// the bipartite graph must be smaller
	if (removalVertex->number != u->number) {
		--sizeofX;
	}

	struct Graph* B = getCachedGraph(sizeofX + sizeofY, cachedB);

	/* store size of first partitioning set */
	B->number = sizeofX;

	/* add vertex numbers of original vertices to ->lowPoint of each vertex in B
	and add edge labels to vertex labels to compare edges easily */
	int i = 0;
	for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->number != removalVertex->number) {
			B->vertices[i]->lowPoint = e->endPoint->number;
			B->vertices[i]->label = e->label;
			++i;
		}
	}
	for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
		B->vertices[i]->lowPoint = e->endPoint->number;
		B->vertices[i]->label = e->label;
		++i;
	}

	/* add edge (x,y) if u in S(y,x) */
	for (i=0; i<sizeofX; ++i) {
		int x = B->vertices[i]->lowPoint;
		for (int j=sizeofX; j<B->n; ++j) {
			int y = B->vertices[j]->lowPoint;

			/* y has to be a child of v */
			if (data.g->vertices[y]->visited < v->visited) {
				/* edge labels have to match, (v, child)->label in g == (u, child)->label in h
				these values were stored in B->vertices[i,j]->label */
				if (labelCmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
					if (containsCharacteristic(data, u, data.h->vertices[x], data.g->vertices[y])) {
						addResidualEdges(B->vertices[i], B->vertices[j], gp->listPool);
						++B->m;
					}
				}
			}
		}
	}
	return B;
}

//
//int computeCharacteristicCached(struct SubtreeIsoDataStore data, struct CachedGraph* cachedB, struct Vertex* y, struct Vertex* u, struct Vertex* v, struct GraphPool* gp) {
//	struct Graph* B = makeBipartiteInstanceFromVerticesCached(data, cachedB, y, u, v, gp);
//	int sizeofMatching = bipartiteMatchingEvenMoreDirty(B);
//
//	int nNeighbors = B->number;
//	returnCachedGraph(cachedB);
//	return (sizeofMatching == nNeighbors) ? 1 : 0;
//}

int computeCharacteristicCached(struct SubtreeIsoDataStore data, struct CachedGraph* cachedB, struct Vertex* y, struct Vertex* u, struct Vertex* v, struct GraphPool* gp) {
	struct Graph* B = makeBipartiteInstanceFromVerticesCached(data, cachedB, y, u, v, gp);
	char hasMatchingCoveringAll = bipartiteMatchingTerminateEarly(B);
	returnCachedGraph(cachedB);
	return hasMatchingCoveringAll;
}


int* getParentsFromPostorder(struct Graph* g, int* postorder) {
	int* parents = malloc(g->n * sizeof(int));
	for (int i=0; i<g->n; ++i) {
		int v = postorder[i];
		parents[v] = g->vertices[v]->lowPoint;
	}
	return parents;
}

/* Return an array holding the indices of the parents of each vertex in g with root root.
the parent of root does not exist, which is indicated by index -1 */
int* getParents(struct Graph* g, int root) {
	int* postorder = getPostorder(g, root);
	int* parents = getParentsFromPostorder(g, postorder);
	free(postorder);
	return parents;
}


// SUBTREE ISOMORPHISM


/**
Iterative Labeled Subtree Isomorphism Check. 

Implements the labeled subtree isomorphism algorithm of
Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism in an iterative version:

Input:
	a text    tree g
	a pattern tree h
	the cube that was computed for some subtree h-e and g, where e is an edge to a leaf of h
	(object pool data structures)

Output:
	yes, if h is subgraph isomorphic to g, no otherwise
	the cube for h and g

 */
static void iterativeSubtreeCheck_intern(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* current, struct GraphPool* gp) {

	struct Graph* g = current->g;
	struct Graph* h = current->h;

	// new vertex and adjacent one
	struct Vertex* b = h->vertices[h->n - 1];
	struct Vertex* a = b->neighborhood->endPoint;

	int* parentsHa = getParents(h, a->number); // move out / rewrite

	struct CachedGraph* cachedB = initCachedGraph(gp, h->n);

	current->foundIso = 0;
	for (int vi=0; vi<g->n; ++vi) {
		struct Vertex* v = g->vertices[current->postorder[vi]];

		// add new characteristics
		if (containsCharacteristic(base, a, a, v)) {
			addCharacteristic(current, b, a, v);
		}
		if (labelCmp(v->label, b->label) == 0) {
			addCharacteristic(current, a, b, v);

			// optimized version of computeCharacteristic(*current, b, b, v, gp);
			for (struct VertexList* e=v->neighborhood; e!= NULL; e=e->next) {
				if (labelCmp(e->label, b->neighborhood->label) == 0) {
					// check if e->endPoint is not the parent of v
					if (e->endPoint->number != v->lowPoint) {
						if (containsCharacteristic(*current, b, a, e->endPoint)) {
							addCharacteristic(current, b, b, v);
							current->foundIso = 1;
							break;
						}
					}
				}
			}
		}

		// filter existing characteristics
		for (int ui=0; ui<h->n-1; ++ui) {
			struct Vertex* u = h->vertices[ui];
#ifndef BITCUBE
#ifdef INTCUBE
			int* oldCharacteristics = rawCharacteristics(base, u, v);
#endif
#ifdef BYTECUBE
			uint8_t* oldCharacteristics = rawCharacteristics(base, u, v);
#endif
			for (int yi=1; yi<=oldCharacteristics[0]; ++yi) {
				struct Vertex* y = h->vertices[oldCharacteristics[yi]];
				if (y->number == parentsHa[u->number]) { // might be a problem for y == a ?
					addCharacteristic(current, y, u, v);
				} else {
					int yuCharacteristic = computeCharacteristicCached(*current, cachedB, y, u, v, gp);
					if (yuCharacteristic) {
						addCharacteristic(current, y, u, v);
						if (y == u) {
							current->foundIso = 1;
						}
					}
				}
			}
#else
			// cache computation. yes, this makes a difference! hottest part of this code is checking if a characteristic exists in base.
			size_t cubeOffset = (v->number * base.h->n + u->number) * base.h->n;
			if (getBit(base.S, cubeOffset + u->number)) {
				int uuCharacteristic = computeCharacteristicCached(*current, cachedB, u, u, v, gp);
				if (uuCharacteristic) {
					addCharacteristic(current, u, u, v);
					current->foundIso = 1;
				}
			}
			for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
				struct Vertex* y = e->endPoint;
				if (y == b) { continue; } // already dealt with above
				if (!getBit(base.S, cubeOffset + y->number)) { continue; } // this is the whole point of this algorithm
				if (y->number == parentsHa[u->number]) { // might be a problem for y == a ?
					addCharacteristic(current, y, u, v);
				} else {
					int yuCharacteristic = computeCharacteristicCached(*current, cachedB, y, u, v, gp);
					if (yuCharacteristic) {
						addCharacteristic(current, y, u, v);
					}
				}
			}
#endif
		}
	}

	free(parentsHa);
	dumpCachedGraph(cachedB);
}


struct SubtreeIsoDataStore iterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct Graph* h, struct GraphPool* gp) {
	struct SubtreeIsoDataStore info = {0};
	info.g = base.g;
	info.h = h;
	info.postorder = base.postorder;

	createNewCubeFromBase(base, &info);
	iterativeSubtreeCheck_intern(base, &info, gp);

	return info;
}


// INITIALIZATORS

struct SubtreeIsoDataStore initG(struct Graph* g) {
	struct SubtreeIsoDataStore info = {0};
	info.postorder = getPostorder(g, 0);
	info.g = g;
	return info;
}


/** create the set of characteristics for a single edge pattern graph */
struct SubtreeIsoDataStore initIterativeSubtreeCheckForEdge(struct SubtreeIsoDataStore base, struct Graph* h) {
	struct SubtreeIsoDataStore info = {0};
	// copy stuff from below
	info.g = base.g;
	info.postorder = base.postorder;

	// create graph from edge
	info.h = h;
	char* edgeLabel = h->vertices[0]->neighborhood->label;

	// create cube
	createNewCubeForEdgePattern(&info);

	int* parents = getParentsFromPostorder(info.g, info.postorder);

	for (int vi=0; vi<(info.g)->n; ++vi) {
		struct Vertex* v = (info.g)->vertices[info.postorder[vi]];
		for (int ui=0; ui<2; ++ui) {
			struct Vertex* u = (info.h)->vertices[ui];
			struct Vertex* y = (info.h)->vertices[(ui + 1) % 2];
			if (labelCmp(v->label, u->label) == 0) {
				// if vertex labels match, there is a characteristic (H^y_u, v)
				addCharacteristic(&info, y, u, v);
				char foundIso = 0;
				// if there is at least one edge that matches and does not lead to the parent, then there is a characteristic (H^u_u, v)
				// we add this characteristic only once below
				for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
					if (parents[v->number] != e->endPoint->number) {
						// check if edge labels match
						if (labelCmp(e->label, edgeLabel) == 0) {
							// if edge does not lead to parent, there is a characteristic (H^u_u, v) if vertex labels of endpoint match
							if (labelCmp(e->endPoint->label, y->label) == 0) {
								foundIso = 1;
							}
						}
					}
				}
				if (foundIso) {
					addCharacteristic(&info, u, u, v);
					info.foundIso = 1;
				}
			}
		}
	}
	free(parents);
	return info;
}

/** create the set of characteristics for a single vertex pattern graph */
struct SubtreeIsoDataStore initIterativeSubtreeCheckForSingleton(struct SubtreeIsoDataStore base, struct Graph* h) {
	struct SubtreeIsoDataStore info = {0};
	// copy stuff from below
	info.g = base.g;
	info.postorder = base.postorder;
	info.h = h;

	struct Vertex* u = h->vertices[0];
	char* uLabel = u->label;

	// create cube
	createNewCubeForSingletonPattern(&info);

	for (int vi=0; vi<(info.g)->n; ++vi) {
		struct Vertex* v = (info.g)->vertices[info.postorder[vi]];

		if (labelCmp(v->label, uLabel) == 0) {
			// if vertex labels match, there is a characteristic (H^y_u, v)
			addCharacteristic(&info, u, u, v);
			info.foundIso = 1;
		}
	}

	return info;
}

/** create the set of characteristics for a single edge pattern graph, given as VertexList. A new graph is created. */
struct SubtreeIsoDataStore initIterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct VertexList* patternEdge, struct GraphPool* gp) {
	// create graph from edge
	struct Graph* h = createGraph(2, gp);
	(h)->vertices[0]->label = patternEdge->startPoint->label;
	(h)->vertices[1]->label = patternEdge->endPoint->label;
	addEdgeBetweenVertices(0, 1, patternEdge->label, h, gp);

	return initIterativeSubtreeCheckForEdge(base, h);

}



/// NONITERATIVE VERSION OF SUBTREE ISO ALGORITHM THAT IS COMPATIBLE WITH ABOVE


void addNoncriticalVertexCharacteristics(struct SubtreeIsoDataStore* data, struct Graph* B, struct Vertex* u, struct Vertex* v) {
	/* the maximum matching computed above covers all but one neighbor of u
	we need to identify those covered neighbors that can be swapped with
	that uncovered neighbor without decreasing the cardinality of the matching
	these are exactly the non-critical vertices.

	a vertex is critical <=> 1.) AND NOT 2.)
	hence
	a vertex is non-critical <=> NOT 1.) OR 2.)
	where
	1.) matched in the matching above
	2.) reachable by augmenting path from the single unmatched vertex.
	This means, all vertices reachable from the single uncovered neighbor of u (including that neighbor are non-critical.
	*/
	struct Vertex* uncoveredNeighbor = NULL;

	// find the single uncovered neighbor of u
	for (int i=0; i<B->number; ++i) {
		if (!isMatched(B->vertices[i])) {
			uncoveredNeighbor = B->vertices[i];
			break;
		}
	}

	// mark all vertices reachable from uncoveredNeighbor by an augmenting path
	markReachable(uncoveredNeighbor, B->number);

	// add non-critical vertices to output
	for (int i=0; i<B->number; ++i) {
		if (B->vertices[i]->visited == 1) {
			// vertex is not critical, add characteristic
			addCharacteristicRaw(data, B->vertices[i]->lowPoint, u->number, v->number);
		}
	}
}

/**
Iterative Labeled Subtree Isomorphism Check.

Implements the labeled subtree isomorphism algorithm of
Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism in an iterative version:

Input:
	a text    tree g
	a pattern tree h
	the cube that was computed for some subtree h-e and g, where e is an edge to a leaf of h
	(object pool data structures)

Output:
	yes, if h is subgraph isomorphic to g, no otherwise
	the cube for h and g

 */
static void noniterativeSubtreeCheck_intern(struct SubtreeIsoDataStore* current, struct GraphPool* gp) {

	struct Graph* g = current->g;
	struct Graph* h = current->h;

	struct CachedGraph* cachedB = initCachedGraph(gp, h->n);

	current->foundIso = 0;
	for (int vi=0; vi<g->n; ++vi) {
		struct Vertex* v = g->vertices[current->postorder[vi]];

		for (int ui=0; ui<h->n; ++ui) {
			struct Vertex* u = h->vertices[ui];

			// check if vertex labels match
			if (labelCmp(u->label, v->label) != 0) { continue; }

			// compute maximum matching
			struct Graph* B = makeBipartiteInstanceFromVerticesCached(*current, cachedB, u, u, v, gp);
			int sizeofMatching = bipartiteMatchingEvenMoreDirty(B);
			int nNeighbors = B->number;

			// is there a subgraph iso here?
			if (sizeofMatching == nNeighbors) {
				addCharacteristic(current, u, u, v);
				current->foundIso = 1;

				returnCachedGraph(cachedB);
				dumpCachedGraph(cachedB);
				return; // early termination when subtree iso is found
			}

			// compute partial subgraph isomorphisms
			if (sizeofMatching == nNeighbors - 1) {
				addNoncriticalVertexCharacteristics(current, B, u, v);
			}

			returnCachedGraph(cachedB);
		}
	}

	dumpCachedGraph(cachedB);
}


struct SubtreeIsoDataStore noniterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct Graph* h, struct GraphPool* gp) {
	struct SubtreeIsoDataStore info = {0};
	info.g = base.g;
	info.h = h;
	info.postorder = base.postorder;

	if (info.g->n > 0) {
		info.S = createNewCube(info.g->n, info.h->n);
		noniterativeSubtreeCheck_intern(&info, gp);
		dumpNewCube(info.S, info.g->n);
	} else {
		// if g is empty, then h only matches if it is empty as well.
		// g->n == 0 is a special case that is not handled well by the subtree iso algorithm
		info.foundIso = h->n == 0 ? 1 : 0;
	}

	info.S = NULL;
	return info;
}

/**
 * Due to historic reasons, this function checks if h is subgraph isomorphic to g.
 */
char isSubtree(struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	struct SubtreeIsoDataStore info = {0};
	info.g = g;
	info.h = h;

	if (info.g->n > 0) {
		info.postorder = getPostorder(g, 0);
		info.S = createNewCube(info.g->n, info.h->n);
		noniterativeSubtreeCheck_intern(&info, gp);
		dumpNewCube(info.S, info.g->n);
		free(info.postorder);
	} else {
		// if g is empty, then h only matches if it is empty as well.
		// g->n == 0 is a special case that is not handled well by the subtree iso algorithm
		info.foundIso = h->n == 0 ? 1 : 0;
	}

	return info.foundIso;
}

