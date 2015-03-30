#include <malloc.h>
#include "graph.h"
#include "bipartiteMatching.h"

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


