#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsomorphism.h"
#include "iterativeSubtreeIsomorphism.h"

// CHARACTERISTICS TOOLING

// TODO can be made constant time
/** Utility data structure creator.
Cube will store, what is called S in the paper. */
int*** createNewCube(int x, int y) {
	int*** cube;
	if ((cube = malloc(x * sizeof(int**)))) {
		for (int i=0; i<x; ++i) {
			cube[i] = malloc(y * sizeof(int*));
			if (cube[i] != NULL) {
				for (int j=0; j<y; ++j) {
					cube[i][j] = NULL;
				}
			} else {
				for (int j=0; j<i; ++j) {
					free(cube[i]);
				}
				free(cube);
				return NULL;
			}
		}
	} else {
		return NULL;
	}
	return cube;
}


int*** createNewCubeFromBase(struct SubtreeIsoDataStore base) {
	int*** S = createNewCube(base.g->n, base.h->n + 1);
	// create cube large enough for filtered characteristics plus new for old vertices of h
	// TODO I assume two things: 
	// 1. we only need to add space for one more characteristic
	// 2. I think, we only need this additional space somewhere, not everywhere. Probably only for the neoghbor of the new vertex
	// 3. the new vertex can have only two characteristics. A complete and an incomplete for the unique parent.
	for (int i=0; i<base.g->n; ++i) {
		for (int j=0; j<base.h->n; ++j) {
			S[i][j] = calloc(base.S[i][j][0] + 2, sizeof(int));
		}
		S[i][base.h->n] = calloc(3, sizeof(int));
	}
	return S;
}

void dumpNewCube(int*** S, int x, int y) {
	for (int i=0; i<x; ++i) {
		for (int j=0; j<y; ++j) {
			free(S[i][j]);
		}
		free(S[i]);
	}
	free(S);
}


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
	int* current = S[v->number][u->number];
	current[0] += 1;
	int newPos = current[0];
	current[newPos] = y->number;
} 


int computeCharacteristic(int*** S, struct Vertex* y, struct Vertex* u, struct Vertex* v, struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	// TODO speedup by handling leaf case separately
	struct Graph* B = makeBipartiteInstanceFromVertices(S, y, u, v, g, h, gp);
	int sizeofMatching = bipartiteMatchingFastAndDirty(B, gp);
	dumpGraph(gp, B);
	return (sizeofMatching == B->number) ? 1 : 0;
}

/** Print a single entry in the cube */
void printNewS(int*** S, int v, int u) {
	int i;
	printf("S(%i, %i)={", v, u);
	if (S[v][u][0] > 0) {
		for (i=1; i<S[v][u][0]; ++i) {
			printf("%i, ", S[v][u][i]);
		}
		printf("%i}\n", S[v][u][S[v][u][0]]);
	} else {
		printf("}\n");
	}
	fflush(stdout);
}

void printNewCube(int*** S, int gn, int hn) {
	for (int i=0; i<gn; ++i) {
		for (int j=0; j<hn; ++j) {
			printNewS(S, i, j);
		}
	}
}



// MISC TOOLING

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
int iterativeSubtreeCheck_intern(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore current, struct GraphPool* gp) {

	struct Graph* g = current.g;
	struct Graph* h = current.h;

	// new vertex and adjacent one
	struct Vertex* b = h->vertices[h->n - 1];
	struct Vertex* a = b->neighborhood->endPoint;

	// int*** newS = createNewCube(g->n, h->n); // rewrite?
	int* parentsHa = getParents(h, a->number); // move out / rewrite

	int foundIso = 0;
	for (int vi=0; vi<g->n; ++vi) {
		struct Vertex* v = g->vertices[current.postorder[vi]];
		
		// add new characteristics TODO check labels
		if (labelCmp(v->label, b->label) == 0) {
			addCharacteristic(current.S, a, b, v);
		}
		if (containsCharacteristic(base.S, a, a, v)) {
			addCharacteristic(current.S, b, a, v);
		}
		int bbCharacteristic = computeCharacteristic(current.S, b, b, v, g, h, gp);
		if (bbCharacteristic) {
			addCharacteristic(current.S, b, b, v);
			foundIso = 1;
		}
		// filter existing characteristics
		for (int ui=0; ui<h->n-1; ++ui) {
			struct Vertex* u = h->vertices[ui];

			if (containsCharacteristic(base.S, u, u, v)) {
				int uuCharacteristic = computeCharacteristic(current.S, u, u, v, g, h, gp);
				if (uuCharacteristic) {
					addCharacteristic(current.S, u, u, v);
					foundIso = 1;
				}
			}
			for (struct VertexList* e=u->neighborhood; e!=NULL; e=e->next) {
				if (containsCharacteristic(base.S, e->endPoint, u, v)) {
					if (e->endPoint->number == parentsHa[u->number]) {
						addCharacteristic(current.S, e->endPoint, u, v);
					} else {
						int yuCharacteristic = computeCharacteristic(current.S, e->endPoint, u, v, g, h, gp);
						if (yuCharacteristic) {
							addCharacteristic(current.S, e->endPoint, u, v);
						}
					}
				}
			}
		}
	}

	free(parentsHa);
	return foundIso;
}


struct SubtreeIsoDataStore iterativeSubtreeCheck(struct SubtreeIsoDataStore base, struct Graph* h, struct GraphPool* gp) {
	struct SubtreeIsoDataStore info = {0};
	info.g = base.g;
	info.h = h;
	info.postorder = base.postorder;
	info.S = createNewCubeFromBase(base);
	info.foundIso = iterativeSubtreeCheck_intern(base, info, gp);
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
	info.S = createNewCube((info.g)->n, 2);
	for (int v=0; v<(info.g)->n; ++v) {
		for (int u=0; u<2; ++u) {
			(info.S)[v][u] = calloc(3, sizeof(int));
		}
	}

	int* parents = getParentsFromPostorder(info.g, info.postorder);

	for (int vi=0; vi<(info.g)->n; ++vi) {
		struct Vertex* v = (info.g)->vertices[info.postorder[vi]];
		for (int ui=0; ui<2; ++ui) {
			struct Vertex* u = (info.h)->vertices[ui];
			struct Vertex* y = (info.h)->vertices[(ui + 1) % 2];
			if (labelCmp(v->label, u->label) == 0) {
				// if vertex labels match, there is a characteristic (H^y_u, v)
				addCharacteristic(info.S, y, u, v);
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
					addCharacteristic(info.S, u, u, v);
					info.foundIso = 1;
				} 
			}
		}
	}
	free(parents);
	return info;
}	

