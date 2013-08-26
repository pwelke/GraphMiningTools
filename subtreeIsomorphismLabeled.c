#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsomorphism.h"
#include "subtreeIsomorphismLabeled.h"


/**
Find all leaves of g that are not equal to r.

The method returns an int array leaves. leaves[0] contains the length of
leaves, i.e. number of leaves plus one.
Subsequent positions of leaves contain the vertex numbers of leaves in ascending order. 
The method sets the ->d members of leaf vertices in g to 1 all other to 0.
*/
int* findLeavesL(struct Graph* g, int root) {
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

	//fprintf(stderr, "make bipartite v=%i u=%i\n", v, u);

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


struct ShallowGraph* removeVertexFromBipartiteInstanceL(struct Graph* B, int v, struct ShallowGraphPool* sgp) {
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


void addVertexToBipartiteInstanceLF(struct ShallowGraph* temp) {
	struct VertexList* e;

	for (e=popEdge(temp); e!=NULL; e=popEdge(temp)) {
		e->used = 0;
		addEdge(e->startPoint, e);
	}
}


void addVertexToBipartiteInstanceL(struct ShallowGraph* temp) {
	struct VertexList* e;

	for (e=popEdge(temp); e!=NULL; e=popEdge(temp)) {
		e->used = 0;
		addEdge(e->startPoint, e);
	}
}


char subtreeCheckL(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* iterators */
	int u, v;

	struct Vertex* r = g->vertices[0];
	int*** S = createCube(g->n, h->n);
	int* postorder = getPostorderL(g, r->number);


	/* init the S(v,u) for v and u leaves */
	int* gLeaves = findLeavesL(g, 0);
	/* h is not rooted, thus every vertex with one neighbor is a leaf */
	int* hLeaves = findLeavesL(h, -1);
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
								struct ShallowGraph* storage = removeVertexFromBipartiteInstanceL(B, i, sgp);
								initBipartite(B);
								matchings[i+1] = bipartiteMatchingFastAndDirty(B, gp);

								if (matchings[i+1] == degU - 1) {
									matchings[i+1] = B->vertices[i]->lowPoint;
								} else {
									matchings[i+1] = -1;
								}

								addVertexToBipartiteInstanceL(storage);
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


char subtreeCheckLF(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* iterators */
	int u, v, w;

	struct Vertex* r = g->vertices[0];
	int*** S = createCube(g->n, h->n);
	int* postorder = getPostorderL(g, r->number);

	// for (v=0; v<g->n; ++v) {
	// 	fprintf(stderr, "%i ", postorder[v]);
	// }
	// fprintf(stderr, "\n");


	/* init the S(v,u) for v and u leaves */
	int* gLeaves = findLeavesL(g, 0);
	/* h is not rooted, thus every vertex with one neighbor is a leaf */
	int* hLeaves = findLeavesL(h, -1);
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
						//fprintf(stderr, "u:%i, v:%i\n", u, postorder[v]);
						int* matchings = malloc((degU + 1) * sizeof(int));
						struct Graph* B = makeBipartiteInstanceL(g, postorder[v], h, u, S, gp);
						
						struct Vertex* s = getVertex(gp->vertexPool);
						struct Vertex* t = getVertex(gp->vertexPool);
						s->number = -1;
						t->number = -2;

						/* Add s, t and edges from s to A and from B to t.
						Also, set residual capacities for these edges correctly */
						for (w=0; w<g->number; ++w) {
							addResidualEdges(s, g->vertices[w], gp->listPool);
						}
						for (w=g->number; w<g->n; ++w) {
							addResidualEdges(g->vertices[w], t, gp->listPool);
						}

						/* find size of matching */
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
								struct ShallowGraph* storage = removeVertexFromBipartiteInstanceLF(B, i, s, sgp);
								struct VertexList* e;
								
								initBipartite(B);
								for (e=s->neighborhood; e!=NULL; e=e->next) {
									setFlag(e, 0);
								}
								for (e=t->neighborhood; e!=NULL; e=e->next) {
									setFlag(e, 1);
								}

								matchings[i+1] = 0;
								while (augment(s, t)) {
									++matchings[i+1];
								}

								if (matchings[i+1] == degU - 1) {
									matchings[i+1] = B->vertices[i]->lowPoint;
								} else {
									matchings[i+1] = -1;
								}

								addVertexToBipartiteInstanceL(storage);
								dumpShallowGraph(sgp, storage);
							} else {
								matchings[i+1] = -1;
							}
						}
						S[current->number][u] = matchings;

						removeSandT(g, s, t, gp);
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
