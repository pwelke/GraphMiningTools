#include <stdlib.h>
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

	
/**
 * Shuffle stack data array. 
 * DONT USE IF STACK IS NOT COMPLETELY FULL!
 *
 * Implementing Fisher–Yates shuffle
 * from http://stackoverflow.com/questions/1519736/random-shuffling-of-an-array
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


/**
Return a random spanning tree of g as a list of its edges. 
Assumes a connected graph g with g->m > 0.
*/
static struct ShallowGraph* _randomSpanningTreeAsShallowGraph(struct Graph* g, struct ShallowGraphPool* sgp) {
	int i, start;
	struct VertexList** previous = malloc(g->n * sizeof(struct VertexList*)); // current random walk
	char* used = malloc(g->n * sizeof(char));
	struct ShallowGraph* tree = getShallowGraph(sgp);
	struct IntegerArrayStack* remaining = initIntegerArrayStack(g->n); // cell indexes to visit
	for (i=0; i<g->n; ++i) {
		ias_push(i, remaining);
		previous[i] = NULL;
		used[i] = 0;
	}
	ias_shuffleArray(remaining);

	// Add a random cell.
	start = ias_pop(remaining);
	used[start] = 1;

	// While there are remaining cells,
	// add a loop-erased random walk to the maze.
	while (!loopErasedRandomWalk(g, remaining, previous, used, tree, sgp->listPool));

	ias_free(remaining);
	free(previous);
	free(used);

	return tree;
}


/**
Return a spanning tree of g as a list of edges. Assumes that g is connected.
*/
struct ShallowGraph* randomSpanningTreeAsShallowGraph(struct Graph* g, struct ShallowGraphPool* sgp) {
	if (g->m == 0) {
		return getShallowGraph(sgp);
	} else {
		return _randomSpanningTreeAsShallowGraph(g, sgp);
	}
}

void eraseWalk(int index0, int index1, struct VertexList** previous) {
	int index;
	for (index=previous[index0]->startPoint->number; 
		index!=index1; 
		index=previous[index0]->startPoint->number) {
		//debug
		// printf("erase ");
		// printEdge(previous[index0]);

		previous[index0] = NULL;
		index0 = index;
	}
	previous[index0] = NULL;
}

struct VertexList* ROOT = (struct VertexList*)0x1;

char loopErasedRandomWalk(struct Graph* g, struct IntegerArrayStack* remaining, struct VertexList** previous, char* used, struct ShallowGraph* tree, struct ListPool* lp) {

	int index0 = -1;
	int index1 = -1;

	// Pick a location that’s not yet in the maze (if any).
	for (index0 = ias_pop(remaining); (index0 != -1) && (used[index0]); index0 = ias_pop(remaining));

	if (index0 == -1) {
		return 1;
	}

	// Perform a random walk starting at this location,
	previous[index0] = ROOT; // must be different from NULL, however, must not be a valid pointer.
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
		if ((previous[index1] != NULL)) {
			eraseWalk(index0, index1, previous);
		} else {
			previous[index1] = e;
		}
		index0 = index1;

		// If this cell is part of the maze, we’re done walking.
		if (used[index1]) {
			// Add the random walk to the maze by backtracking to the starting cell.
			// Also erase this walk’s history to not interfere with subsequent walks.
			for (index0=previous[index1]->startPoint->number; index0!=index1; index0=previous[index1]->startPoint->number) {
				used[index0] = 1;
				appendEdge(tree, shallowCopyEdge(previous[index1], lp));
				previous[index1] = NULL;
				index1 = index0;
				if (previous[index1] == ROOT) {
					break;
				}
			}
			previous[index1] = NULL;
			return 0;
		}
	}
}
