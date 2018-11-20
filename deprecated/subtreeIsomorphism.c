#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsoUtils.h"
#include "subtreeIsomorphism.h"

int*** _cube = NULL;
int _cubeX = 0;
int _cubeY = 0;
char _cubeInUse = 0;

/** Utility data structure destructor */
static void _freeCube(int*** cube, int x, int y) {
	int i, j;
	for (i=0; i<x; ++i) {
		if (cube[i] != NULL) {
			for (j=0; j<y; ++j) {
				if (cube[i][j] != NULL) {
					free(cube[i][j]);
				}
			}
			free(cube[i]);
		}
	}
	free(cube);
}

void dumpCube() {
	_freeCube(_cube, _cubeX, _cubeY);
}


/** Utility data structure creator.
Cube will store, what is called S in the paper. */
static int*** createCube(int x, int y) {
	if (!_cubeInUse) {
		int i, j;
		if ((x > _cubeX) || (y > _cubeY)) {
			if (_cube != NULL) {
				_freeCube(_cube, _cubeX, _cubeY);
			}
			_cubeX = x;
			_cubeY = y;
			if ((_cube = malloc(x * sizeof(int**)))) {
				for (i=0; i<x; ++i) {
					_cube[i] = malloc(y * sizeof(int*));
					if (_cube[i] != NULL) {
						for (j=0; j<y; ++j) {
							_cube[i][j] = NULL;
						}
					} else {
						for (j=0; j<i; ++j) {
							free(_cube[i]);
						}
						free(_cube);
						return NULL;
					}
				}
			} else {
				return NULL;
			}
			_cubeInUse = 1;
			return _cube;
		} else {
			_cubeInUse = 1;
			return _cube;
		}
	} else {
		return NULL;
	}
}



/** Utility data structure destructor */
static void freeCube(int*** cube, int x, int y) {
	int i, j;
	for (i=0; i<x; ++i) {
		if (cube[i] != NULL) {
			for (j=0; j<y; ++j) {
				if (cube[i][j] != NULL) {
					free(cube[i][j]);
					cube[i][j] = NULL;
				}
			}
		}
	}
	_cubeInUse = 0;
}


/**
Find all leaves of g that are not equal to r.

The method returns an int array leaves. leaves[0] contains the length of
leaves, i.e. number of leaves plus one.
Subsequent positions of leaves contain the vertex numbers of leaves in ascending order. 
The method sets the ->d members of leaf vertices in g to 1 all other to 0.
*/
static int* findLeaves(struct Graph* g, int root) {
	int nLeaves = 0;
	int* leaves;
	int v;

	for (v=0; v<g->n; ++v) {
		if (v != root) {
			if (isLeaf(g->vertices[v])) {
				++nLeaves;
				g->vertices[v]->d = 1;
			} else {
				g->vertices[v]->d = 0;
			}	
		}
	}
	leaves = malloc((nLeaves+1) * sizeof(int));
	leaves[0] = nLeaves + 1;
	nLeaves = 0;
	for (v=0; v<g->n; ++v) {
		if (v != root) {
			if (isLeaf(g->vertices[v])) {
				leaves[nLeaves+1] = v;
				++nLeaves;	
			}
		}
	}
	return leaves;
}


/* vertices of g have their ->visited values set to the postorder. Thus, 
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
static struct Graph* makeBipartiteInstance(struct Graph* g, int v, struct Graph* h, int u, int*** S, struct GraphPool* gp) {
	struct Graph* B;
	int i, j;

	int sizeofX = degree(h->vertices[u]);
	int sizeofY = degree(g->vertices[v]);
	struct VertexList* e;

 	/* construct bipartite graph B(v,u) */ 
	B = createGraph(sizeofX + sizeofY, gp);
	/* store size of first partitioning set */
	B->number = sizeofX;

	/* add vertex numbers of original vertices to ->lowPoint of each vertex in B 
	TODO add edge labels to vertex labels to compare edges easily */
	i = 0;
	for (e=h->vertices[u]->neighborhood; e!=NULL; e=e->next) {
		B->vertices[i]->lowPoint = e->endPoint->number;
		B->vertices[i]->label = e->label;
		++i;
	}
	for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
		B->vertices[i]->lowPoint = e->endPoint->number;
		B->vertices[i]->label = e->label;
		++i;
	}

	/* add edge (x,y) if u in S(y,x) */
	for (i=0; i<sizeofX; ++i) {
		for (j=sizeofX; j<B->n; ++j) {	
			int x = B->vertices[i]->lowPoint;
			int y = B->vertices[j]->lowPoint;
			int k;

			/* y has to be a child of v */
			if (g->vertices[y]->visited < g->vertices[v]->visited) {
				/* vertex labels have to match */
				if (labelCmp(g->vertices[y]->label, h->vertices[x]->label) == 0) {
					/* edge labels have to match, (v, child)->label in g == (u, child)->label in h 
					these values were stored in B->vertices[i,j]->label */
					if (labelCmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
						if (S[y][x] != NULL) {
							for (k=1; k<S[y][x][0]; ++k) {
								if (S[y][x][k] == u) {
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


/**
 * deprecated. use isSubtree from module iterativeSubtreeIsomorphism
 *
Labeled Subtree isomorphism check. 

Implements the version of subtree isomorphism algorithm described by

Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism. 

Section 2 and Section 3 in the labeled version.
It differs from the other subtreeCheck versions by just computing a single matching and then computing
critical vertices by a simple augmenting path property check.
*/
char subtreeCheck3(struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	/* iterators */
	int u, v;

	struct Vertex* r = g->vertices[0];
	int*** S = createCube(g->n, h->n);
	int* postorder = getPostorder(g, r->number);

	/* init the S(v,u) for v and u leaves */
	int* gLeaves = findLeaves(g, 0);
	/* h is not rooted, thus every vertex with one neighbor is a leaf */
	int* hLeaves = findLeaves(h, -1);
	for (v=1; v<gLeaves[0]; ++v) {
		for (u=1; u<hLeaves[0]; ++u) {
			/* check compatibility of leaf labels */
			if (labelCmp(g->vertices[gLeaves[v]]->label, h->vertices[hLeaves[u]]->label) == 0) {
				/* check for compatibility of edges */
				if (labelCmp(g->vertices[gLeaves[v]]->neighborhood->label, h->vertices[hLeaves[u]]->neighborhood->label) == 0) {
					S[gLeaves[v]][hLeaves[u]] = malloc(2 * sizeof(int));
					/* 'header' of array stores its length */
					S[gLeaves[v]][hLeaves[u]][0] = 2;
					/* the number of the unique neighbor of u in h*/
					S[gLeaves[v]][hLeaves[u]][1] = h->vertices[hLeaves[u]]->neighborhood->endPoint->number;
				}
			}
		}
	}
	/* garbage collection for init */
	free(gLeaves);
	free(hLeaves);
	gLeaves = NULL;
	hLeaves = NULL;

	for (v=0; v<g->n; ++v) {
		struct Vertex* current = g->vertices[postorder[v]];
		int currentDegree = degree(current);
		if ((currentDegree > 1) || (current->number == r->number)) {
			for (u=0; u<h->n; ++u) {
				int i;
				int degU = degree(h->vertices[u]);
				if (degU <= currentDegree + 1) {
					/* if vertex labels match */
					if (labelCmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstance(g, current->number, h, u, S, gp);
						int* matchings = malloc((degU + 1) * sizeof(int));

						// matchings[0] = bipartiteMatchingFastAndDirty(B, gp);
						matchings[0] = bipartiteMatchingEvenMoreDirty(B);

						// have we found a subgraph isomorphism?
						if (matchings[0] == degU) {
							free(postorder);
							free(matchings);
							freeCube(S, g->n, h->n);
							dumpGraph(gp, B);
							return 1;
						} 

						// check if it makes sense to search for critical vertices
						if (matchings[0] == degU - 1) {
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
							for (i=0; i<B->number; ++i) {
								if (!isMatched(B->vertices[i])) {
									uncoveredNeighbor = B->vertices[i];
									break;
								}			
							}

							// mark all vertices reachable from uncoveredNeighbor by an augmenting path
							markReachable(uncoveredNeighbor, B->number);

							// add non-critical vertices to output
							matchings[0] = degU + 1;
							for (i=0; i<B->number; ++i) {
								if (B->vertices[i]->visited == 1) {
									// vertex is not critical
									matchings[i+1] = B->vertices[i]->lowPoint;
								} else {
									// vertex critical
									matchings[i+1] = -1;
								}
							}

						} else {
							// makes no sense to look for critical vertices as there cannot be a matching covering all but one neighbor of u
							matchings[0] = degU + 1;
							for (i=1; i<degU + 1; ++i) {
								matchings[i] = -1;
							}
						}

						// store information for further steps of the algorithm
						S[current->number][u] = matchings;
						// garbage collection
						dumpGraph(gp, B);
					}
				}
			}		
		}
	}

	/* garbage collection */
	free(postorder);
	freeCube(S, g->n, h->n);

	return 0;
}


/**
Labeled Rooted Subtree isomorphism check.

This is a variant of the subtree isomorphism algorithm (for the undirected case) described by

Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism.

(see also subtreeCheck3 for the implementation of the undirected case.)

This variant here is probably less efficient than a specialized variant for the rooted case, but was easier to implement:
- it checks whether a complete subtree isomorphism from h to g maps the root of h to the highest point in g.
- if so, it returns a pointer to the vertex in g, o/w, it returns NULL.

This allows to on the one hand decide whether there exists a rooted iso from h to g with given roots, and on the
other hand, to reconstruct the mapping from h to g (or at least have a hint about it).

Input:
tree g, rooted at gRoot
tree h, rooted at hRoot

Output:
a pointer to v \in V(g) such that there exists a rooted subtree iso from h rooted at hRoot to g rooted at gRoot that
  maps hRoot to v, NULL otherwise.
*/
struct Vertex* subtreeCheckRooted(struct Graph* g, int gRoot, struct Graph* h, int hRoot, struct GraphPool* gp) {
	/* iterators */
	int u, v;

	struct Vertex* r = g->vertices[gRoot];
	int*** S = createCube(g->n, h->n);
	int* postorder = getPostorder(g, r->number);

	/* init the S(v,u) for v and u leaves */
	int* gLeaves = findLeaves(g, 0);
	/* h is not rooted, thus every vertex with one neighbor is a leaf */
	int* hLeaves = findLeaves(h, -1);
	for (v=1; v<gLeaves[0]; ++v) {
		for (u=1; u<hLeaves[0]; ++u) {
			/* check compatibility of leaf labels */
			if (labelCmp(g->vertices[gLeaves[v]]->label, h->vertices[hLeaves[u]]->label) == 0) {
				/* check for compatibility of edges */
				if (labelCmp(g->vertices[gLeaves[v]]->neighborhood->label, h->vertices[hLeaves[u]]->neighborhood->label) == 0) {
					S[gLeaves[v]][hLeaves[u]] = malloc(2 * sizeof(int));
					/* 'header' of array stores its length */
					S[gLeaves[v]][hLeaves[u]][0] = 2;
					/* the number of the unique neighbor of u in h*/
					S[gLeaves[v]][hLeaves[u]][1] = h->vertices[hLeaves[u]]->neighborhood->endPoint->number;
				}
			}
		}
	}
	/* garbage collection for init */
	free(gLeaves);
	free(hLeaves);
	gLeaves = NULL;
	hLeaves = NULL;

	for (v=0; v<g->n; ++v) {
		struct Vertex* current = g->vertices[postorder[v]];
		int currentDegree = degree(current);
		if ((currentDegree > 1) || (current->number == r->number)) {
			for (u=0; u<h->n; ++u) {
				int i;
				int degU = degree(h->vertices[u]);
				if (degU <= currentDegree + 1) {
					/* if vertex labels match */
					if (labelCmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstance(g, current->number, h, u, S, gp);
						int* matchings = malloc((degU + 1) * sizeof(int));

						// matchings[0] = bipartiteMatchingFastAndDirty(B, gp);
						matchings[0] = bipartiteMatchingEvenMoreDirty(B);

						// have we found a subgraph isomorphism?
						if ((matchings[0] == degU) && (u == hRoot)) {
							free(postorder);
							free(matchings);
							freeCube(S, g->n, h->n);
							dumpGraph(gp, B);
							return g->vertices[v];
						}

						// check if it makes sense to search for critical vertices
						if (matchings[0] == degU - 1) {
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
							for (i=0; i<B->number; ++i) {
								if (!isMatched(B->vertices[i])) {
									uncoveredNeighbor = B->vertices[i];
									break;
								}
							}

							// mark all vertices reachable from uncoveredNeighbor by an augmenting path
							markReachable(uncoveredNeighbor, B->number);

							// add non-critical vertices to output
							matchings[0] = degU + 1;
							for (i=0; i<B->number; ++i) {
								if (B->vertices[i]->visited == 1) {
									// vertex is not critical
									matchings[i+1] = B->vertices[i]->lowPoint;
								} else {
									// vertex critical
									matchings[i+1] = -1;
								}
							}

						} else {
							// makes no sense to look for critical vertices as there cannot be a matching covering all but one neighbor of u
							matchings[0] = degU + 1;
							for (i=1; i<degU + 1; ++i) {
								matchings[i] = -1;
							}
						}

						// store information for further steps of the algorithm
						S[current->number][u] = matchings;
						// garbage collection
						dumpGraph(gp, B);
					}
				}
			}
		}
	}

	/* garbage collection */
	free(postorder);
	freeCube(S, g->n, h->n);

	return NULL;
}

