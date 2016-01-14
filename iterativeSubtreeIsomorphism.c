#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsomorphism.h"
// #include "iterativeSubtreeIsomorphism.h"

struct SubtreeIsoDataStore {
	int* postorder;
	struct Graph* g;
	struct Graph* h;
	int*** S;
	int foundIso;
};

struct SubtreeIsoDataStore initG(struct Graph* g) {
	struct SubtreeIsoDataStore info = {0};
	info.postorder = getPostorder(g, 0);
	info.g = g;
	return info;
}

/** create the set of characteristics for s single edge pattern graph */
struct SubtreeIsoDataStore initIterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct VertexList* e, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct SubtreeIsoDataStore info = {0};
	// copy stuff from below
	info.g = base.g;
	info.postorder = base.postorder;

	// create graph from edge
	info.h = createGraph(2, gp);
	(info.h)->vertices[0]->label = e->startPoint->label;
	(info.h)->vertices[1]->label = e->endPoint->label;
	addEdgeBetweenVertices(0, 1, e->label, info.h, gp);

	// compute characteristics
	info.S = createCube((info.g)->n, 2);

	return info;
}	

/* vertices of g have their ->visited values set to the postorder. Thus, 
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
struct Graph* makeBipartiteInstanceFromVertices(int*** S, struct Vertex* removalVertex, struct Vertex* u, struct Vertex* v, struct Graph* g, struct Graph* h, struct GraphPool* gp) {
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
			if (g->vertices[y]->visited < v->visited) {
				/* vertex labels have to match */
				if (labelCmp(g->vertices[y]->label, h->vertices[x]->label) == 0) {
					/* edge labels have to match, (v, child)->label in g == (u, child)->label in h 
					these values were stored in B->vertices[i,j]->label */
					if (labelCmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
						if (S[y][x] != NULL) {
							for (int k=1; k<=S[y][x][0]; ++k) {
								if (S[y][x][k] == u->number) {
									addResidualEdges(B->vertices[i], B->vertices[j], gp->listPool);
									++B->m;
								}
							}
						}
					}
				}
			}
		}
	} 

	return B;
}

/* Return an array holding the indices of the parents of each vertex in g with root root.
the parent of root does not exist, which is indicated by index -1 */
int* getParents(struct Graph* g, int root) {
	int* postorder = getPostorder(g, root);
	int* parents = malloc(g->n * sizeof(int));
	for (int i=0; i<g->n; ++i) {
		int v = postorder[i];
		parents[v] = g->vertices[v]->lowPoint;
	}
	free(postorder);
	return parents;
}

// TODO can be made constant time
int containsCharacteristic(int*** S, struct Vertex* y, struct Vertex* u, struct Vertex* v) {
	int uvNumberOfCharacteristics = S[v->number][u->number][0];
	for (int i=1; i<=uvNumberOfCharacteristics; ++i) {
		if (y->number == S[v->number][u->number][i]) {
			return 1;
		}
	}
	return 0;
}

// assumes that 
void addCharacteristic(int*** S, struct Vertex* y, struct Vertex* u, struct Vertex* v) {
	S[v->number][u->number][0] += 1;
	int newPos = S[v->number][u->number][0];
	S[v->number][u->number][newPos] = y->number;
} 


int computeCharacteristic(int*** S, struct Vertex* y, struct Vertex* u, struct Vertex* v, struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	// TODO speedup by handling leaf case separately
	struct Graph* B = makeBipartiteInstanceFromVertices(S, y, u, v, g, h, gp);
	int sizeofMatching = bipartiteMatchingFastAndDirty(B, gp);
	dumpGraph(gp, B);
	return (sizeofMatching == B->number) ? 1 : 0;
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
char iterativeSubtreeCheck(struct Graph* g, struct Graph* h, struct Vertex* newVertex, int*** oldS, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* supressWarning = getShallowGraph(sgp);
	if (supressWarning) {}

	// newVertex is a leaf, hence there is only one incident edge, which is the newly added one.
	struct VertexList* newEdge = newVertex->neighborhood;
	struct Vertex* a = newEdge->endPoint;
	struct Vertex* b = newEdge->startPoint;

	struct Vertex* r = g->vertices[0];
	int*** newS = createCube(g->n, h->n); // rewrite?
	int* postorder = getPostorder(g, r->number); // move out
	int* parentsHa = getParents(h, a->number); // move out / rewrite

	char foundIso = 0;
	for (int vi=0; vi<g->n; ++vi) {
		struct Vertex* v = g->vertices[postorder[vi]];
		for (int ui=0; ui<h->n; ++ui) {
			struct Vertex* u = h->vertices[ui];

			// add new characteristics
			addCharacteristic(newS, a, b, v);
			if (containsCharacteristic(oldS, a, a, v)) {
				addCharacteristic(newS, b, a, v);
			}
			int bbCharacteristic = computeCharacteristic(newS, b, b, v, g, h, gp);
			if (bbCharacteristic) {
				addCharacteristic(newS, b, b, v);
				foundIso = 1;
			}

			// filter existing characteristics
			if (containsCharacteristic(oldS, u, u, v)) {
				int uuCharacteristic = computeCharacteristic(newS, u, u, v, g, h, gp);
				if (uuCharacteristic) {
					addCharacteristic(newS, u, u, v);
					foundIso = 1;
				}
			}
			for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
				if (containsCharacteristic(oldS, e->endPoint, u, v)) {
					if (e->endPoint->number == parentsHa[u->number]) {
						addCharacteristic(newS, e->endPoint, u, v);
					} else {
						int yuCharacteristic = computeCharacteristic(newS, e->endPoint, u, v, g, h, gp);
						if (yuCharacteristic) {
							addCharacteristic(newS, e->endPoint, u, v);
						}
					}
				}
			}
		}
	}

	free(parentsHa);
	return foundIso;
}
