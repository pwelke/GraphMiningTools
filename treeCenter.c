#include <malloc.h>
#include <limits.h>

#include "graph.h"
#include "canonicalString.h"
#include "treeCenter.h"

int markEdges(struct Vertex* root, int value) {
	struct VertexList* e;
	int max = value;

	root->lowPoint = value;

	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->lowPoint > value) {
			int current = markEdges(e->endPoint, value + 1);
			e->used = current;
			if (current > max) {
				max = current;
			}
		}
	}

	return max;
}

void moveCenter(struct Vertex* root, struct Vertex* parent, int value, int* centers) {
	struct VertexList* e;
	int max = INT_MIN;
	struct VertexList* max1;
	struct VertexList* max2;
	struct VertexList* parentEdge = NULL;

	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint != parent) {
			e->used -= value;
		} else {
			parentEdge = e;
		}
	}

	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->used > max) {
			max = e->used;
			max1 = e;
		}
	}

	max = INT_MIN;
	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e != max1) {
			if (e->used > max) {
				max = e->used;
				max2 = e;
			}
		}
	}

	if (max1->used == max2->used) {
		centers[0] = 2;
		centers[1] = root->number;
		return;
	}

	if (max1->used == max2->used + 1) {
		centers[0] = 3;
		centers[1] = root->number;
		centers[2] = max1->endPoint->number;
		return;
	}

	if (max2->used == max1->used + 1) {
		centers[0] = 3;
		centers[1] = root->number;
		centers[2] = max2->endPoint->number;
		return;
	}

	if (parentEdge != NULL) {
		parentEdge->used = max2->used + 1;
	}
	moveCenter(max1->endPoint, root, value + 1, centers);
}

int* treeCenter(struct Graph* tree) {
	int* centers = malloc(3 * sizeof(int));
	int i;


	for (i=0; i<tree->n; ++i) {
		tree->vertices[i]->lowPoint = INT_MAX;
	}

	markEdges(tree->vertices[0], 0);

	moveCenter(tree->vertices[0], NULL, 0, centers);

	return centers;
}

struct ShallowGraph* treeCenterCanonicalString(struct Graph* tree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* cString;
	int* center = treeCenter(tree);

	cString = canonicalStringOfRootedTree(tree->vertices[center[1]], tree->vertices[center[1]], sgp);
	
	if (center[0] == 3) {
		struct ShallowGraph* cString2 = canonicalStringOfRootedTree(tree->vertices[center[2]], tree->vertices[center[2]], sgp);
		if (lexicographicComparison(cString, cString2) < 0) {
			dumpShallowGraph(sgp, cString2);
		} else {
			dumpShallowGraph(sgp, cString);
			cString = cString2;
		}
	}
	free(center);
	return cString;
}
