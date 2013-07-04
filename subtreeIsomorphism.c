#include <malloc.h>
#include "graph.h"



struct RootedTree {
	struct Vertex* root;
	int n;
};

struct Table {
	void* data;
	int itemSize;
	int n;
	int m;
};

struct Table* getTable(int n, int m, int size) {
	struct Table* table = malloc(sizeof(struct Table));
	table->data = malloc(n * m * size);
	table->n = n;
	table->m = m;
	table->itemSize = size;
	return table;
}

int dumpTable(struct Table* table) {
	free(table->data);
	free(table);
}

void* getValue(struct Table* table, int i, int j) {
	int pos = (i * table->n + j) * table->itemSize;
	/* cast to char to have pointer arithmetic for one byte steps */
	return &(((char*)table->data)[pos]);
}

void setValue(struct Table* table, int i, int j, int value) {
	int pos = (i * table->n + j) * table->itemSize;
	/* cast to char to have pointer arithmetic for one byte steps */
	((char*)table->data)[pos] = value;
} 

int*** createCube(int x, int y) {
	int*** cube;
	int i, j;
	if (cube = malloc(x * sizeof(int**))) {
		for (i=0; i<x; ++i) {
			cube[i] = malloc(y * sizeof(int*));
			if (cube[i] != NULL) {
				for (int j=0; j<y; ++j) {
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

char isleaf(struct Vertex* v) {
	// check if v has a neigbor at all
	if (v->neighborhood) {
		// check if v has exactly one neighbor, thus is a leaf
		if (v->neighborhood->next == NULL) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

int* findLeaves(struct Graph* g, int root) {
	int nLeaves = 0;
	int* leaves;
	int v;

	for (v=0; v<g->n; ++v) {
		if (v != root) {
			if (isLeaf(g->vertices[v]) {
				++nLeaves;
			}		
		}
	}
	leaves = malloc((nLeaves+1) * sizeof(int));
	leaves[0] = nLeaves + 1;
	nLeaves = 0;
	for (v=0; v<g->n; ++v) {
		if (v != root) {
			// if g is a tree with more than one vertex, this should always be true. 
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

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->visited == -1) {
			value = 1 + dfs(e->endPoint, value);
		}
	}
	v->visited = value;
	return value;
}

int* getPostorder(struct Graph* g, int root) {
	int i;
	int* order = malloc(g->n * sizeof(int));
	for (int i=0; i<g->n; ++i) {
		g->vertices[i]->visited = -1;
	}
	dfs(root, 0);
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]->visited] != -1) {
			order[g->vertices[i]->visited] = i;
		} else {
			// debug
			fprintf(stderr, "Vertex %i was not visited by dfs\n", i);
		}
	}
	return order;
}

struct Graph* makeBipartiteInstance() {}


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
		if (isLeaf(current) || (current->number == root->number)) {

		}
	}
	return 0;
}

