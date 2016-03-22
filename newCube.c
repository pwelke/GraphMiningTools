#include <assert.h>
#include <malloc.h>

#include "newCube.h"

// CHARACTERISTICS TOOLING

// TODO can be made constant time
/** Utility data structure creator.
Cube will store, what is called S in the paper. */
static int*** createNewCube_intern(int x, int y) {
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


void createNewCubeForEdgePattern(struct SubtreeIsoDataStore* info) {
	info->S = createNewCube_intern(info->g->n, 2);
	info->elementsInS = 0;
	int* array = malloc(3 * 2 * info->g->n * sizeof(int));
	size_t position = 0;
	for (int v=0; v<(info->g)->n; ++v) {
		for (int u=0; u<2; ++u) {
			int* newPos = array + position;
			(info->S)[v][u] = newPos; // pointer arithmetic
			(info->S)[v][u][0] = 0;
			position += 3;
		}
	}
}

void createNewCubeForSingletonPattern(struct SubtreeIsoDataStore* info) {
	info->S = createNewCube_intern(info->g->n, 1);
	info->elementsInS = 0;
	int* array = malloc(2 * info->g->n * sizeof(int));
	size_t position = 0;
	for (int v=0; v<(info->g)->n; ++v) {
		int* newPos = array + position;
		(info->S)[v][0] = newPos; // pointer arithmetic
		(info->S)[v][0][0] = 0;
		position += 2;
	}
}


void createNewCubeFromBase(struct SubtreeIsoDataStore base, struct SubtreeIsoDataStore* new) {

	// create cube large enough for filtered characteristics plus new for old vertices of h
	// TODO I assume two things:
	// 1. we only need to add space for one more characteristic
	// 2. I think, we only need this additional space somewhere, not everywhere. Probably only for the neoghbor of the new vertex
	// 3. the new vertex can have only two characteristics. A complete and an incomplete for the unique parent.
	size_t maxSize = base.elementsInS; // space for characteristics of subgraph
	maxSize += base.g->n * (base.h->n + 1); // space for storing size info
	maxSize += base.g->n * base.h->n; // space for additional characteristic
	maxSize += 2 * base.g->n; // space for characteristics of new vertex
	int* array = calloc(maxSize, sizeof(int));
	size_t position = 0;

	int x = base.g->n;
	int y = base.h->n + 1;
	int*** cube;
	if ((cube = malloc(x * sizeof(int**)))) {
		for (int i=0; i<x; ++i) {
			cube[i] = malloc(y * sizeof(int*));
			if (cube[i] != NULL) {
				for (int j=0; j<y; ++j) {
					cube[i][j] = &(array[position]);
					if (j < y - 1) {
						position += base.S[i][j][0] + 2;
					} else {
						position += 3;
					}
				}
			} else {
				for (int j=0; j<i; ++j) {
					free(cube[i]);
				}
				free(cube);
				new->S = NULL;
			}
		}
	} else {
		new->S = NULL;
	}
	new->S = cube;
	new->elementsInS = 0;

}


void dumpNewCube(int*** S, int x) {
	free(S[0][0]);
	for (int i=0; i<x; ++i) {
		free(S[i]);
	}
	free(S);
}


int* rawCharacteristics(struct SubtreeIsoDataStore data, struct Vertex* u, struct Vertex* v) {
	return data.S[v->number][u->number];
}


int containsCharacteristic(struct SubtreeIsoDataStore data, struct Vertex* y, struct Vertex* u, struct Vertex* v) {
	for (int i=1; i<=data.S[v->number][u->number][0]; ++i) {
		if (y->number == data.S[v->number][u->number][i]) {
			return 1;
		}
	}
	return 0;
}


char checkSanityOfWrite(struct SubtreeIsoDataStore* data, struct Vertex* u, struct Vertex* v) {
	// find next position in cube
	int nextV;
	int nextU;
	if (u->number < data->h->n - 1) {
		nextV = v->number;
		nextU = u->number + 1;
	} else {
		nextV = v->number + 1;
		nextU = 0;
	}

	// last position in cube
	if (nextV == data->g->n) {
		// should have two positions
		int storedElements = data->S[v->number][u->number][0];
		if (storedElements < 2) {
			return 1;
		} else {
			return 0;
		}

	} else {
		int storedElements = data->S[v->number][u->number][0];
		int space = data->S[nextV][nextU] - data->S[v->number][u->number];
		if (storedElements < space - 1) {
			return 1;
		} else {
			return 0;
		}
	}
}


// assumes that
void addCharacteristic(struct SubtreeIsoDataStore* data, struct Vertex* y, struct Vertex* u, struct Vertex* v) {
	assert(checkSanityOfWrite(data, u, v));
	data->elementsInS += 1;
	int* current = data->S[v->number][u->number];
	current[0] += 1;
	int newPos = current[0];
	current[newPos] = y->number;
	//	} else {
	//		fprintf(stderr, "Invalid write to cube: y=%i u=%i v=%i.\nIs: ", y->number, u->number, v->number);
	//		printNewS(data->S, v->number, u->number);
	//	}
}


/** Print a single entry in the cube */
void printNewCubeRow(int*** S, int v, int u) {
	int i;
	printf("S(%i, %i)={", v, u);
	int* row = S[v][u];
	int count = row[0];
	if (count > 0) {
		for (i=1; i<count; ++i) {
			printf("%i, ", row[i]);
		}
		printf("%i}\n", row[count]);
	} else {
		printf("}\n");
	}
	fflush(stdout);

}


void printNewSDanger(int* data, size_t length) {
	for (size_t i=0; i<length; ++i) {
		printf("%i, ", data[i]);
	}
	printf("\n");
}


void printNewCube(int*** S, int gn, int hn) {
	for (int i=0; i<gn; ++i) {
		for (int j=0; j<hn; ++j) {
			printNewCubeRow(S, i, j);
		}
	}
}


void printNewCubeCondensed(int*** S, int gn, int hn) {
	for (int i=0; i<gn; ++i) {
		for (int j=0; j<hn; ++j) {
			if (S[i][j][0] != 0) {
				printNewCubeRow(S, i, j);
			}
		}
	}
}


void testNewCubeSizes(int*** S, int gn, int hn) {
	int size = 0;
	for (int i=0; i<gn; ++i) {
		for (int j=0; j<hn; ++j) {
			size += S[i][j][0];
		}
	}
	printf("has %i entries\n", size);
}
