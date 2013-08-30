#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsomorphism.h"
#include "cachedGraph.h"
#include "subtreeIsomorphismLabeled.h"

int dfsL(struct Vertex* v, int value) {
	struct VertexList* e;

	/* to make this method save for graphs that are not trees */
	v->visited = -2;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->visited == -1) {
			value = 1 + dfsL(e->endPoint, value);
			e->endPoint->lowPoint = v->number;
		}
	}
	v->visited = value;
	return value;
}


/**
Compute a dfsL order or postorder on g starting at vertex root.
The ->visited members of vertices are set to the position they have in the order 
(starting with 0). Vertices that cannot be reached from root get ->visited = -1
The method returns an array of length g->n where position i contains the vertex number 
of the ith vertex in the order. 
The ->lowPoint s of vertices in g point to their parents in the postorder.
*/
int* getPostorderL(struct Graph* g, int root) {
	int i;
	int* order = malloc(g->n * sizeof(int));
	for (i=0; i<g->n; ++i) {
		g->vertices[i]->visited = -1;
		order[i] = -1;
	}
	dfsL(g->vertices[root], 0);
	g->vertices[root]->lowPoint = -1;
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]->visited != -1) {
			order[g->vertices[i]->visited] = i;
		} else {
			/* should never happen if g is a tree */
			fprintf(stderr, "Vertex %i was not visited by dfsL.\nThis can not happen, if g is a tree.\n", i);
		}
	}
	return order;
}


/* vertices of g have their ->visited values set to the postorder. Thus, 
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
struct Graph* makeBipartiteInstanceL(struct Graph* g, int v, struct Graph* h, int u, int*** S, struct GraphPool* gp) {
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
				if (strcmp(g->vertices[y]->label, h->vertices[x]->label) == 0) {
					/* edge labels have to match, (v, child)->label in g == (u, child)->label in h 
					these values were stored in B->vertices[i,j]->label */
					if (strcmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
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
struct Graph* makeBipartiteInstanceLF(struct Graph* g, int v, struct Graph* h, int u, int*** S, struct CachedGraph* cache) {
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
				if (strcmp(g->vertices[y]->label, h->vertices[x]->label) == 0) {
					/* edge labels have to match, (v, child)->label in g == (u, child)->label in h 
					these values were stored in B->vertices[i,j]->label */
					if (strcmp(B->vertices[i]->label, B->vertices[j]->label) == 0) {
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


struct ShallowGraph* removeVertexFromBipartiteInstanceLF(struct Graph* B, int v, struct Vertex* s, struct ShallowGraphPool* sgp) {
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
char subtreeCheckL(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* iterators */
	int u, v;

	struct Vertex* r = g->vertices[0];
	int*** S = createCube(g->n, h->n);
	int* postorder = getPostorderL(g, r->number);


	/* init the S(v,u) for v and u leaves */
	int* gLeaves = findLeaves(g, 0);
	/* h is not rooted, thus every vertex with one neighbor is a leaf */
	int* hLeaves = findLeaves(h, -1);
	for (v=1; v<gLeaves[0]; ++v) {
		for (u=1; u<hLeaves[0]; ++u) {
			/* check compatibility of leaf labels */
			if (strcmp(g->vertices[gLeaves[v]]->label, h->vertices[hLeaves[u]]->label) == 0) {
				/* check for compatibility of edges */
				if (strcmp(g->vertices[gLeaves[v]]->neighborhood->label, h->vertices[hLeaves[u]]->neighborhood->label) == 0) {
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
					if (strcmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstanceL(g, postorder[v], h, u, S, gp);
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
								&& (strcmp(h->vertices[B->vertices[i]->lowPoint]->label, g->vertices[current->lowPoint]->label) == 0)) {
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
Labeled Subtree isomorphism check. 
Speedup to method above by reusing the whole bipartite matching utility graph B (including s and t) 

Implements the version of subtree isomorphism algorithm described by

Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism. Section 2 

in the labeled version
*/
char subtreeCheckLF(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* iterators */
	int u, v;

	struct Vertex* r = g->vertices[0];
	int*** S = createCube(g->n, h->n);
	int* postorder = getPostorderL(g, r->number);

	struct CachedGraph* cacheB = initCachedGraph(gp, g->n);


	/* init the S(v,u) for v and u leaves */
	int* gLeaves = findLeaves(g, 0);
	/* h is not rooted, thus every vertex with one neighbor is a leaf */
	int* hLeaves = findLeaves(h, -1);
	for (v=1; v<gLeaves[0]; ++v) {
		for (u=1; u<hLeaves[0]; ++u) {
			/* check compatibility of leaf labels */
			if (strcmp(g->vertices[gLeaves[v]]->label, h->vertices[hLeaves[u]]->label) == 0) {
				/* check for compatibility of edges */
				if (strcmp(g->vertices[gLeaves[v]]->neighborhood->label, h->vertices[hLeaves[u]]->neighborhood->label) == 0) {
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
					if (strcmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstanceLF(g, postorder[v], h, u, S, cacheB);
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
								&& (strcmp(h->vertices[B->vertices[i]->lowPoint]->label, g->vertices[current->lowPoint]->label) == 0)) {
								
								struct VertexList* e;

								/* remove vertex i from B and init B for matching algorithm */
								struct ShallowGraph* storage = removeVertexFromBipartiteInstanceLF(B, i, s, sgp);
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
						//dumpGraph(gp, B);
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
	int* postorder = getPostorderL(g, r->number);

	/* init the S(v,u) for v and u leaves */
	int* gLeaves = findLeaves(g, 0);
	/* h is not rooted, thus every vertex with one neighbor is a leaf */
	int* hLeaves = findLeaves(h, -1);
	for (v=1; v<gLeaves[0]; ++v) {
		for (u=1; u<hLeaves[0]; ++u) {
			/* check compatibility of leaf labels */
			if (strcmp(g->vertices[gLeaves[v]]->label, h->vertices[hLeaves[u]]->label) == 0) {
				/* check for compatibility of edges */
				if (strcmp(g->vertices[gLeaves[v]]->neighborhood->label, h->vertices[hLeaves[u]]->neighborhood->label) == 0) {
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
					if (strcmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstanceLF(g, postorder[v], h, u, S, cacheB);
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
								&& (strcmp(h->vertices[B->vertices[i]->lowPoint]->label, g->vertices[current->lowPoint]->label) == 0)) {
								
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


char subtreeCheckLFF(struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	struct CachedGraph* cacheB = initCachedGraph(gp, g->n);
	char isSubtree = subtreeCheckCached(g, h, gp, cacheB);
	dumpCachedGraph(cacheB);
	return isSubtree;
}
