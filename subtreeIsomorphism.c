#include <malloc.h>
#include "graph.h"

int*** createCube(int x, int y) {
	int*** cube;
	int i, j;
	if ((cube = malloc(x * sizeof(int**)))) {
		for (i=0; i<x; ++i) {
			cube[i] = malloc(y * sizeof(int*));
			if (cube[i] != NULL) {
				for (j=0; j<y; ++j) {
					cube[i][j] = NULL;
				}
			} else {
				for (j=0; j<i; ++j) {
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

void freeCube(int*** cube, int x, int y) {
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

char isLeaf(struct Vertex* v) {
	/* check if v has a neigbor at all */
	if (v->neighborhood) {
		/* check if v has exactly one neighbor, thus is a leaf */
		if (v->neighborhood->next == NULL) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
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

int dfs(struct Vertex* v, int value) {
	struct VertexList* e;

	/* to make this method save for graphs that are not trees */
	v->visited = -2;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->visited == -1) {
			value = 1 + dfs(e->endPoint, value);
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
*/
int* getPostorder(struct Graph* g, int root) {
	int i;
	int* order = malloc(g->n * sizeof(int));
	for (i=0; i<g->n; ++i) {
		g->vertices[i]->visited = -1;
		order[i] = -1;
	}
	dfs(g->vertices[root], 0);
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]->visited != -1) {
			order[g->vertices[i]->visited] = i;
		} else {
			// debug
			fprintf(stderr, "Vertex %i was not visited by dfs\n", i);
		}
	}
	return order;
}

/* vertices of g have their ->visited values set to the postorder. Thus, 
children of v are vertices u that are neighbors of v and have u->visited < v->visited */
struct Graph* makeBipartiteInstance(struct Graph* g, int v, struct Graph* h, int u, int*** S, struct GraphPool* gp) {
	struct Graph* B;

	int sizeofX = 0;
	int sizeofY = 0;
	struct VertexList* e;

	/* get size of neighborhoods to construct graph */
	int orderofV = g->vertices[v]->visited;
	for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->visited < orderofV) {
			++sizeofX;
		}
	}
	for (e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
		++sizeofY;
	}

 	/* construct bipartite graph B(v,u) */ 
	B = createGraph(sizeofX + sizeofY, gp);

	return B;
}


char subtreeCheck(struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	// iterators
	struct VertexList* e;
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
			S[gLeaves[v]][hLeaves[u]] = malloc(2 * sizeof(int));
			/* 'header' of array stores its length */
			S[gLeaves[v]][hLeaves[u]][0] = 2;
			/* the number of the unique neighbor of u in h*/
			S[gLeaves[v]][hLeaves[u]][1] = h->vertices[hLeaves[u]]->neighborhood->endPoint->number;
		}
	}
	/* garbage collection for init */
	free(gLeaves);
	free(hLeaves);

	for (v=0; v<g->n; ++v) {
		struct Vertex* current = g->vertices[postorder[v]];
		if (isLeaf(current) || (current->number == r->number)) {

		}
	}
	return 0;
}

