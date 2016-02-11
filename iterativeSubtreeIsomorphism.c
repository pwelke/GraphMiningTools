#include <malloc.h>
#include <string.h>
#include <stdlib.h>
//#include <assert.h>

#include "newCube.h"
#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsomorphism.h"
#include "iterativeSubtreeIsomorphism.h"
#include "bitSet.h"



// MISC TOOLING
int computeCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v, struct GraphPool* gp) {
	// TODO speedup by handling leaf case separately
	struct Graph* B = makeBipartiteInstanceFromVertices(data, y, u, v, gp);
	int sizeofMatching = bipartiteMatchingFastAndDirty(B, gp);
	dumpGraph(gp, B);
	return (sizeofMatching == B->number) ? 1 : 0;
}


/* vertices of g have their ->visited values set to the postorder. Thus,
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
struct Graph* makeBipartiteInstanceFromVertices(struct SubtreeIsoDataStore data, struct Vertex* removalVertex, struct Vertex* u, struct Vertex* v, struct GraphPool* gp) {
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
		for (int j=sizeofX; j<B->n; ++j) {
			int x = B->vertices[i]->lowPoint;
			int y = B->vertices[j]->lowPoint;

			/* y has to be a child of v */
			if (data.g->vertices[y]->visited < v->visited) {
				/* vertex labels have to match */
				if (labelCmp(data.g->vertices[y]->label, data.h->vertices[x]->label) == 0) {
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
	}

	return B;
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
void iterativeSubtreeCheck_intern(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* current, struct GraphPool* gp) {

	struct Graph* g = current->g;
	struct Graph* h = current->h;

	// new vertex and adjacent one
	struct Vertex* b = h->vertices[h->n - 1];
	struct Vertex* a = b->neighborhood->endPoint;

	int* parentsHa = getParents(h, a->number); // move out / rewrite

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

			if (containsCharacteristic(base, u, u, v)) {
				int uuCharacteristic = computeCharacteristic(*current, u, u, v, gp);
				if (uuCharacteristic) {
					addCharacteristic(current, u, u, v);
					current->foundIso = 1;
				}
			}
			for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
				if (containsCharacteristic(base, e->endPoint, u, v)) {
					if (e->endPoint->number == parentsHa[u->number]) {
						addCharacteristic(current, e->endPoint, u, v);
					} else {
						int yuCharacteristic = computeCharacteristic(*current, e->endPoint, u, v, gp);
						if (yuCharacteristic) {
							addCharacteristic(current, e->endPoint, u, v);
						}
					}
				}
			}
		}
	}

	free(parentsHa);
}


struct SubtreeIsoDataStore iterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct Graph* h, struct GraphPool* gp) {
	struct SubtreeIsoDataStore info = {0};
	info.g = base.g;
	info.h = h;
	info.postorder = base.postorder;

	createNewCubeFromBaseFast(base, &info);
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
struct SubtreeIsoDataStore initIterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct VertexList* patternEdge, struct GraphPool* gp) {
	struct SubtreeIsoDataStore info = {0};
	// copy stuff from below
	info.g = base.g;
	info.postorder = base.postorder;

	// create graph from edge
	info.h = createGraph(2, gp);
	(info.h)->vertices[0]->label = patternEdge->startPoint->label;
	(info.h)->vertices[1]->label = patternEdge->endPoint->label;
	addEdgeBetweenVertices(0, 1, patternEdge->label, info.h, gp);

	// create cube
	createNewBaseCubeFast(&info);

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
						if (labelCmp(e->label, patternEdge->label) == 0) {
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

