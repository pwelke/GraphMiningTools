#ifndef WILSONS_ALGORITHM_H_
#define WILSONS_ALGORITHM_H_

#include "graph.h"

/**
 * A stack of integer values that uses an array of fixed size. 
 * An empty stack is indicated by pop() returning -1
 * 
 * If something tries to fill more elements in the stack than possible, 
 * an ArrayIndexOutOfBoundsException is thrown.

 	Transcoded from Java
 * 
 * @throws ArrayIndexOutOfBoundsException if the stack is full.
 * @author pascal
 *
 */
struct IntegerArrayStack {
	int* stackData;
	int top;
	int capacity;
};

struct ArrayList {
	int** stackData;
	int top;
	int capacity;
};

struct ShallowGraph* randomSpanningTreeAsShallowGraph(struct Graph* g, struct ShallowGraphPool* sgp);
char loopErasedRandomWalk(struct Graph* g, struct IntegerArrayStack* remaining, int* previous, char* used, struct ShallowGraph* tree, struct ListPool* lp);
#endif