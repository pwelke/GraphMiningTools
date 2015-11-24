#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsomorphism.h"

int*** _cube = NULL;
int _cubeX = 0;
int _cubeY = 0;
char _cubeInUse = 0;

/** Utility data structure destructor */
void _freeCube(int*** cube, int x, int y) {
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
int*** createCube(int x, int y) {
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
void freeCube(int*** cube, int x, int y) {
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



/** Print a single entry in the cube */
void printS(int*** S, int v, int u) {
	int i;
	printf("S(%i, %i)={", v, u);
	if (S[v][u]) {
		for (i=1; i<S[v][u][0]-1; ++i) {
			printf("%i, ", S[v][u][i]);
		}
		printf("%i}\n", S[v][u][S[v][u][0]-1]);
	} else {
		printf("}\n");
	}
	fflush(stdout);
}


/**
 * Print some information about a ShallowGraph
 */
void printStrangeMatching(struct ShallowGraph* g) {
	
	struct ShallowGraph* index = g;
	struct VertexList* e;
	do {
		if (index) {
			printf("matching ");
			for (e=index->edges; e; e=e->next) {
				printf("(%i, %i) ", e->startPoint->lowPoint, e->endPoint->lowPoint);
			}
			printf("\n");
		} else {
			/* if index is NULL, the input pointed to a list and not to a cycle */
			break;
		}
	} while (index != g);
}


/**
Find all leaves of g that are not equal to r.

The method returns an int array leaves. leaves[0] contains the length of
leaves, i.e. number of leaves plus one.
Subsequent positions of leaves contain the vertex numbers of leaves in ascending order. 
The method sets the ->d members of leaf vertices in g to 1 all other to 0.
*/
int* findLeaves(struct Graph* g, int root) {
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






struct ShallowGraph* removeVertexFromBipartiteInstance(struct Graph* B, int v, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* temp = getShallowGraph(sgp);
	struct VertexList* e;
	struct VertexList* f;
	struct VertexList* g;	
	int w;


	/* mark edges that will be removed */
	for (e=B->vertices[v]->neighborhood; e!=NULL; e=e->next) {
		e->used = 1;
		((struct VertexList*)e->label)->used = 1;
	}

	/* remove edges from v */
	for (e=B->vertices[v]->neighborhood; e!=NULL; e=f) {
		f = e->next;
		appendEdge(temp, e);
	}
	B->vertices[v]->neighborhood = NULL;

	/* remove residual edges */
	for (w=B->number; w<B->n; ++w) {
		f = NULL;
		g = NULL;
		/* partition edges */
		for (e=B->vertices[w]->neighborhood; e!=NULL; e=B->vertices[w]->neighborhood) {
			B->vertices[w]->neighborhood = e->next;
			if (e->used == 1) {
				e->next = f;
				f = e;
			} else {
				e->next = g;
				g = e;
			}
		}
		/* set neighborhood to unused, append used to temp */
		B->vertices[w]->neighborhood = g;
		while (f!=NULL) {
			e = f;
			f = f->next;
			appendEdge(temp, e);
		}
	}
	return temp;
}

void addVertexToBipartiteInstance(struct ShallowGraph* temp) {
	struct VertexList* e;

	for (e=popEdge(temp); e!=NULL; e=popEdge(temp)) {
		e->used = 0;
		addEdge(e->startPoint, e);
	}
}


/**
Wrapper for strcmp that can deal with NULL strings.
We fix the semantic of a vertex or edge label that is NULL as follows:
It matches every label and is matched by every label. 
I.e. labelCmp always returns 0 if one of the arguments is NULL.
*/
int labelCmp(const char* l1, const char* l2) {
	return ((l1 == NULL) || (l2 == NULL)) ? 0 : strcmp(l1, l2);
}

int dfs(struct Vertex* v, int value) {
	struct VertexList* e;

	/* to make this method save for graphs that are not trees */
	v->visited = -2;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->visited == -1) {
			value = 1 + dfs(e->endPoint, value);
			e->endPoint->lowPoint = v->number; 
		}
	}
	v->visited = value;
	return value;
}


/**
Compute a dfs order or postorder on g starting at vertex root.
The ->visited members of vertices are set to the position they have in the order 
(starting with 0). Vertices that cannot be reached from root get ->visited = -1
The method returns an array of length g->n where position i contains the vertex number 
of the ith vertex in the order. 
The ->lowPoint s of vertices in g point to their parents in the postorder.
*/
int* getPostorder(struct Graph* g, int root) {
	int i;
	int* order = malloc(g->n * sizeof(int));
	for (i=0; i<g->n; ++i) {
		g->vertices[i]->visited = -1;
		order[i] = -1;
	}
	dfs(g->vertices[root], 0);
	g->vertices[root]->lowPoint = -1; 
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]->visited != -1) {
			order[g->vertices[i]->visited] = i;
		} else {
			/* should never happen if g is a tree */
			fprintf(stderr, "Vertex %i was not visited by dfs.\nThis can not happen, if g is a tree.\n", i);
		}
	}
	return order;
}


/* vertices of g have their ->visited values set to the postorder. Thus, 
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
struct Graph* makeBipartiteInstance(struct Graph* g, int v, struct Graph* h, int u, int*** S, struct GraphPool* gp) {
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


/* vertices of g have their ->visited values set to the postorder. Thus, 
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
struct Graph* makeBipartiteInstanceF(struct Graph* g, int v, struct Graph* h, int u, int*** S, struct CachedGraph* cache) {
	struct Graph* B;
	int i, j;

	int sizeofX = degree(h->vertices[u]);
	int sizeofY = degree(g->vertices[v]);
	struct VertexList* e;

 	/* construct bipartite graph B(v,u) */ 
	B = getCachedGraph(sizeofX + sizeofY, cache);
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
									addResidualEdges(B->vertices[i], B->vertices[j], cache->gp->listPool);
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


struct ShallowGraph* removeVertexFromBipartiteInstanceF(struct Graph* B, int v, struct Vertex* s, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* temp = getShallowGraph(sgp);
	struct VertexList* e;
	struct VertexList* f;
	struct VertexList* g;	
	int w;


	/* mark edges that will be removed */
	for (e=B->vertices[v]->neighborhood; e!=NULL; e=e->next) {
		e->used = 1;
		((struct VertexList*)e->label)->used = 1;
	}

	/* remove edges from v */
	for (e=B->vertices[v]->neighborhood; e!=NULL; e=f) {
		f = e->next;
		appendEdge(temp, e);
	}
	B->vertices[v]->neighborhood = NULL;

	/* remove residual edges */
	for (w=B->number; w<B->n; ++w) {
		f = NULL;
		g = NULL;
		/* partition edges */
		for (e=B->vertices[w]->neighborhood; e!=NULL; e=B->vertices[w]->neighborhood) {
			B->vertices[w]->neighborhood = e->next;
			if (e->used == 1) {
				e->next = f;
				f = e;
			} else {
				e->next = g;
				g = e;
			}
		}
		/* set neighborhood to unused, append used to temp */
		B->vertices[w]->neighborhood = g;
		while (f!=NULL) {
			e = f;
			f = f->next;
			appendEdge(temp, e);
		}
	}

	/* find edge (s,v) */
	f = NULL;
	g = NULL;
	/* partition edges */
	for (e=s->neighborhood; e!=NULL; e=s->neighborhood) {
		s->neighborhood = e->next;
		if (e->used == 1) {
			e->next = f;
			f = e;
		} else {
			e->next = g;
			g = e;
		}
	}
	/* set neighborhood to unused, append used to temp */
	s->neighborhood = g;
	while (f!=NULL) {
		e = f;
		f = f->next;
		appendEdge(temp, e);
	}
	return temp;
}


/**
Labeled Subtree isomorphism check. 

Implements the version of subtree isomorphism algorithm described by

Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism. Section 2 

in the labeled version
*/
char subtreeCheck(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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
		if ((currentDegree != 1) || (current->number == r->number)) {
			for (u=0; u<h->n; ++u) {
				int i;
				int degU = degree(h->vertices[u]);
				if (degU <= currentDegree + 1) {
					/* if vertex labels match */
					if (labelCmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstance(g, postorder[v], h, u, S, gp);
						int* matchings = malloc((degU + 1) * sizeof(int));

						matchings[0] = bipartiteMatchingFastAndDirty(B, gp);

						if (matchings[0] == degU) {
							free(postorder);
							free(matchings);
							freeCube(S, g->n, h->n);
							dumpGraph(gp, B);
							return 1;
						} else {
							matchings[0] = degU + 1;
						}

						for (i=0; i<B->number; ++i) {
							/* if the label of ith child of u is compatible to the label of the parent of v */
							if ((current->lowPoint != -1) 
								&& (labelCmp(h->vertices[B->vertices[i]->lowPoint]->label, g->vertices[current->lowPoint]->label) == 0)) {
								struct ShallowGraph* storage = removeVertexFromBipartiteInstance(B, i, sgp);
								initBipartite(B);
								matchings[i+1] = bipartiteMatchingFastAndDirty(B, gp);

								if (matchings[i+1] == degU - 1) {
									matchings[i+1] = B->vertices[i]->lowPoint;
								} else {
									matchings[i+1] = -1;
								}

								addVertexToBipartiteInstance(storage);
								dumpShallowGraph(sgp, storage);
							} else {
								matchings[i+1] = -1;
							}
						}
						S[current->number][u] = matchings;

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
Find all vertices reachable by augmenting paths that start with a non-matching edge
*/
static void markReachable(struct Vertex* a) {
	struct VertexList* e;

	a->visited = 1;
	for (e=a->neighborhood; e!=NULL; e=e->next) {
		if ((e->flag == 0) && (e->endPoint->visited == 0)) {
			markReachable(e->endPoint);
		}
	}
}


/**
Labeled Subtree isomorphism check. 

Implements the version of subtree isomorphism algorithm described by

Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism. 

Section 2 and Section 3 in the labeled version.
It differs from the other subtreeCheck versions by just computing a single matching and then computing
critical vertices by a simple augmenting path property check.
*/
char subtreeCheck3(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* iterators */
	int u, v;

	struct Vertex* r = g->vertices[0];
	int*** S = createCube(g->n, h->n);
	int* postorder = getPostorder(g, r->number);
	printf("Postorder:");
	for (v=0; v<g->n; ++v) printf(" %i", postorder[v]);
	printf("\n");


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

						matchings[0] = bipartiteMatchingFastAndDirty(B, gp);
						printf("for vertex current = %i and u = %i\n", current->number, u);
						printShallowGraph(getMatching(B, sgp));

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

							a vertex is critical <=> 1.) AND 2.)
							1.) matched in the matching above
							2.) not reachable by augmenting path from the single unmatched vertex. */
							struct Vertex* uncoveredNeighbor = NULL;
							
							// find the single uncovered neighbor of u
							for (i=0; i<B->number; ++i) {
								if (!isMatched(B->vertices[i])) {
									uncoveredNeighbor = B->vertices[i];
									break;
								}			
							}

							// mark all vertices reachable from uncoveredNeighbor by an augmenting path
							markReachable(uncoveredNeighbor);
							// unmark the uncovered neighbor itself, as it is not critical
							uncoveredNeighbor->visited = 0; 

							// add non-critical vertices to output
							matchings[0] = degU + 1;
							for (i=0; i<B->number; ++i) {
								if (B->vertices[i]->visited == 1) {
									// vertex critical
									matchings[i+1] = -1;
								} else {
									// vertex is not critical
									matchings[i+1] = B->vertices[i]->lowPoint;
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
Labeled Subtree isomorphism check. 
Speedup to method above by reusing the whole bipartite matching utility graph B (including s and t) 

Implements the version of subtree isomorphism algorithm described by

Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism. Section 2 

in the labeled version
*/
char subtreeCheckF(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* iterators */
	int u, v;

	struct Vertex* r = g->vertices[0];
	int*** S = createCube(g->n, h->n);
	int* postorder = getPostorder(g, r->number);

	struct CachedGraph* cacheB = initCachedGraph(gp, g->n);


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
		if ((currentDegree != 1) || (current->number == r->number)) {
			for (u=0; u<h->n; ++u) {
				int i;
				int degU = degree(h->vertices[u]);
				if (degU <= currentDegree + 1) {
					/* if vertex labels match */
					if (labelCmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstanceF(g, postorder[v], h, u, S, cacheB);
						int* matchings = malloc((degU + 1) * sizeof(int));

						int w;
						struct Vertex* s = getVertex(gp->vertexPool);
						struct Vertex* t = getVertex(gp->vertexPool);
						s->number = -1;
						t->number = -2;
						
						/* Add s, t and edges from s to A and from B to t.
						Also, set residual capacities for these edges correctly */
						for (w=0; w<B->number; ++w) {
							addResidualEdges(s, B->vertices[w], gp->listPool);
						}

						for (w=B->number; w<B->n; ++w) {
							addResidualEdges(B->vertices[w], t, gp->listPool);
						}

						matchings[0] = 0;
						while (augment(s, t)) {
							++matchings[0];
						}

						if (matchings[0] == degU) {
							free(postorder);
							free(matchings);
							freeCube(S, g->n, h->n);
							dumpGraph(gp, B);
							return 1;
						} else {
							matchings[0] = degU + 1;
						}

						for (i=0; i<B->number; ++i) {
							/* if the label of ith child of u is compatible to the label of the parent of v */
							if ((current->lowPoint != -1) 
								&& (labelCmp(h->vertices[B->vertices[i]->lowPoint]->label, g->vertices[current->lowPoint]->label) == 0)) {
								
								struct VertexList* e;

								/* remove vertex i from B and init B for matching algorithm */
								struct ShallowGraph* storage = removeVertexFromBipartiteInstanceF(B, i, s, sgp);
								initBipartite(B);
								for (e=s->neighborhood; e!=NULL; e=e->next) {
									setFlag(e, 0);
								}
								for (e=t->neighborhood; e!=NULL; e=e->next) {
									setFlag(e, 1);
								}
								
								/* run the matching algorithm */
								matchings[i+1] = 0;	
								while (augment(s, t)) {
									++matchings[i+1];
								}

								/* convert output to S(u,v) format */
								if (matchings[i+1] == degU - 1) {
									matchings[i+1] = B->vertices[i]->lowPoint;
								} else {
									matchings[i+1] = -1;
								}

								addVertexToBipartiteInstance(storage);
								dumpShallowGraph(sgp, storage);
							} else {
								matchings[i+1] = -1;
							}
						}
						S[current->number][u] = matchings;
						
						/* remove s and t, dump B */
						dumpVertexListRecursively(gp->listPool, s->neighborhood);
						dumpVertexListRecursively(gp->listPool, t->neighborhood);
						dumpVertex(gp->vertexPool, s);
						dumpVertex(gp->vertexPool, t);
						returnCachedGraph(cacheB);
					}
				}
			}		
		}
	}

	/* garbage collection */
	free(postorder);
	freeCube(S, g->n, h->n);
	dumpCachedGraph(cacheB);

	return 0;
}


/**
dfs that searches for a path from s to t and augments it, 
if found.
returns 1 if there is a path or 0 otherwise.
*/
char augmentLFF(struct Vertex* s, struct Vertex* t) {
	struct VertexList* e;

	if (s == t) {
		return 1;
	}
	s->visited = 1;
	for (e=s->neighborhood; e!=NULL; e=e->next) {
		if ((e->flag == 0) && (e->endPoint->visited == 0) && (e->endPoint->d == 0)) {
			char found = augment(e->endPoint, t);
			if (found) {
				setFlag(e, 1);
				s->visited = 0;
				return 1;
			}
		}
	}
	s->visited = 0;
	return 0;
}


/**
Labeled Subtree isomorphism check. 
Speedup to method above by reusing the whole bipartite matching utility graph B (including s and t) 

Implements the version of subtree isomorphism algorithm described by

Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism. Section 2 

in the labeled version
*/
char subtreeCheckCached(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct CachedGraph* cacheB) {
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
		if ((currentDegree != 1) || (current->number == r->number)) {
			for (u=0; u<h->n; ++u) {
				int i;
				int degU = degree(h->vertices[u]);
				if (degU <= currentDegree + 1) {
					/* if vertex labels match */
					if (labelCmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstanceF(g, postorder[v], h, u, S, cacheB);
						int* matchings = malloc((degU + 1) * sizeof(int));

						int w;
						struct Vertex* s = getVertex(gp->vertexPool);
						struct Vertex* t = getVertex(gp->vertexPool);
						s->number = -1;
						t->number = -2;
						
						/* Add s, t and edges from s to A and from B to t.
						Also, set residual capacities for these edges correctly */
						for (w=0; w<B->number; ++w) {
							addResidualEdges(s, B->vertices[w], gp->listPool);
						}

						for (w=B->number; w<B->n; ++w) {
							addResidualEdges(B->vertices[w], t, gp->listPool);
						}

						matchings[0] = 0;
						while (augmentLFF(s, t)) {
							++matchings[0];
						}

						if (matchings[0] == degU) {
							free(postorder);
							free(matchings);
							freeCube(S, g->n, h->n);
							/* remove s and t, dump B */
							dumpVertexListRecursively(gp->listPool, s->neighborhood);
							dumpVertexListRecursively(gp->listPool, t->neighborhood);
							dumpVertex(gp->vertexPool, s);
							dumpVertex(gp->vertexPool, t);
							returnCachedGraph(cacheB);
							return 1;
						} else {
							matchings[0] = degU + 1;
						}

						for (i=0; i<B->number; ++i) {
							/* if the label of ith child of u is compatible to the label of the parent of v */
							if ((current->lowPoint != -1) 
								&& (labelCmp(h->vertices[B->vertices[i]->lowPoint]->label, g->vertices[current->lowPoint]->label) == 0)) {
								
								struct VertexList* e;

								/* remove vertex i from B and init B for matching algorithm */
								B->vertices[i]->d = 1;
								initBipartite(B);
								for (e=s->neighborhood; e!=NULL; e=e->next) {
									setFlag(e, 0);
								}
								for (e=t->neighborhood; e!=NULL; e=e->next) {
									setFlag(e, 1);
								}
								
								/* run the matching algorithm */
								matchings[i+1] = 0;	
								while (augmentLFF(s, t)) {
									++matchings[i+1];
								}

								/* convert output to S(u,v) format */
								if (matchings[i+1] == degU - 1) {
									matchings[i+1] = B->vertices[i]->lowPoint;
								} else {
									matchings[i+1] = -1;
								}
								/* add vertex i to bipartite instance */
								B->vertices[i]->d = 0;
							} else {
								matchings[i+1] = -1;
							}
						}
						S[current->number][u] = matchings;
						
						/* remove s and t, dump B */
						dumpVertexListRecursively(gp->listPool, s->neighborhood);
						dumpVertexListRecursively(gp->listPool, t->neighborhood);
						dumpVertex(gp->vertexPool, s);
						dumpVertex(gp->vertexPool, t);
						returnCachedGraph(cacheB);
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


char subtreeCheckFF(struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	struct CachedGraph* cacheB = initCachedGraph(gp, g->n);
	char isSubtree = subtreeCheckCached(g, h, gp, cacheB);
	dumpCachedGraph(cacheB);
	return isSubtree;
}
