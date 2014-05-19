#include <stdlib.h>
#include <malloc.h>
#include "wilsonsAlgorithm.h"
#include "graphPrinting.h" 


struct IntegerArrayStack* initIntegerArrayStack(int capacity) {
	struct IntegerArrayStack* ias = malloc(sizeof(struct IntegerArrayStack));
	ias->stackData = malloc(capacity * sizeof(int));
	ias->top = -1;
	ias->capacity = capacity;
	return ias;
}

void ias_push(int value, struct IntegerArrayStack* ias) {
	ias->stackData[++ias->top] = value;
}

int ias_pop(struct IntegerArrayStack* ias) {
	if (ias->top != -1) {
		return ias->stackData[ias->top--];
	} else {
		return -1;
	}
}

void ias_free(struct IntegerArrayStack* ias) {
	if (ias->stackData) free(ias->stackData);
	free (ias);
}

	// Implementing Fisher–Yates shuffle
	// from http://stackoverflow.com/questions/1519736/random-shuffling-of-an-array
	/**
	 * Shuffle stack data array. 
	 * DONT USE IF STACK IS NOT COMPLETELY FULL!
	 */
void ias_shuffleArray(struct IntegerArrayStack* ias) {
	int i;
	for (i = ias->capacity - 1; i > 0; i--)
	{
		int index = rand() % (i + 1);
		// Simple swap
		int a = ias->stackData[index];
		ias->stackData[index] = ias->stackData[i];
		ias->stackData[i] = a;
	}
}

struct ArrayList* initArrayList(int capacity) {
	struct ArrayList* al = malloc(sizeof(struct ArrayList));
	al->stackData = malloc(capacity * sizeof(int*));
	al->top = -1;
	al->capacity = capacity;
	return al;
}

void al_push(int* value, struct ArrayList* al) {
	al->stackData[++al->top] = value;
}

int* al_pop(struct ArrayList* al) {
	if (al->top != -1) {
		return al->stackData[al->top--];
	} else {
		return NULL;
	}
}

void al_free(struct ArrayList* al) {
	if (al->stackData) free(al->stackData);
	free (al);
}



struct ShallowGraph* randomSpanningTreeAsShallowGraph(struct Graph* g, struct ShallowGraphPool* sgp) {
	int i, start;
	int* previous = malloc(g->n * sizeof(struct VertexList*)); // current random walk
	char* used = malloc(g->n * sizeof(char));
	struct ShallowGraph* tree = getShallowGraph(sgp);
	struct IntegerArrayStack* remaining = initIntegerArrayStack(g->n); // cell indexes to visit
	for (i=0; i<g->n; ++i) {
		ias_push(i, remaining);
		previous[i] = NULL;
		used[i] = 0;
	}
	ias_shuffleArray(remaining);

	//debug
	fprintf(stderr, "entering wilson\n");

	// Add a random cell.
	start = ias_pop(remaining);
	used[start] = 1;

	// While there are remaining cells,
	// add a loop-erased random walk to the maze.
	while (!loopErasedRandomWalk(g, remaining, previous, used, tree, sgp->listPool));

	// treeEdges = tree->stackData;
	
	// tree->stackData = NULL;
	// al_free(tree);
	free(previous);
	free(used);

	return tree;
}

void eraseWalk(int index0, int index1, struct VertexList* previous) {
	int index;
	for (index = previous[index0]->endPoint; index != index1; index = previous[index0]->endPoint) {
		previous[index0] = -1;
		index0 = index;
	}
	previous[index0] = -1;
}

char loopErasedRandomWalk(struct Graph* g, struct IntegerArrayStack* remaining, int* previous, char* used, struct ShallowGraph* tree, struct ListPool* lp) {

	int index0 = -1;
	int index1 = -1;

	//debug
	fprintf(stderr, "entering loopE... ");


	// Pick a location that’s not yet in the maze (if any).
	for (index0 = ias_pop(remaining); (index0 != -1) && (used[index0]); index0 = ias_pop(remaining)) {
		// //debug
		// fprintf(stderr, "index0 %i is used\n", index0);
	}

	//debug
	fprintf(stderr, "use vertex %i \n", index0);

	if (index0 == -1) {
		return 1;
	}

	// Perform a random walk starting at this location,
	previous[index0] = index0;
	while (1) {
		struct VertexList* e;
		int neighborIndex = rand() % degree(g->vertices[index0]);
		int i = 0;
		for (e=g->vertices[index0]->neighborhood; e!=NULL; e=e->next) {
			if (i == neighborIndex) {
				index1 = e->endPoint->number;
				break;
			} else {
				++i;
			}
		}
		// e is set to the edge selected

		// If this new cell was visited previously during this walk,
		// erase the loop, rewinding the path to its earlier state.
		// Otherwise, just add it to the walk.
		if (previous[index1] >= 0) {
			eraseWalk(index0, index1, previous);
		} else {
			previous[index1] = index0;
		}
		index0 = index1;

		// If this cell is part of the maze, we’re done walking.
		if (used[index1]) {

			// Add the random walk to the maze by backtracking to the starting cell.
			// Also erase this walk’s history to not interfere with subsequent walks.
			for (index0=previous[index1]; index0!=index1; index0=previous[index1]) {
				//debug
				fprintf(stderr, "adding (%i, %i) to tree\n", index0, index1);

				used[index0] = 1;
				appendEdge(tree, shallowCopyEdge(e, lp));
				previous[index1] = -1;
				index1 = index0;
			}
			previous[index1] = -1;
			return 0;
		}
	}
}
