#ifndef LIST_CYCLES_H_
#define LIST_CYCLES_H_

#include "graph.h"

char findPath(struct Vertex* v, struct Vertex* parent, struct Vertex* target, int allowance, struct ShallowGraph* path, struct ShallowGraphPool *sgp);
struct ShallowGraph* listCycles(struct Graph *g, struct ShallowGraphPool *sgp);

int getNumberOfSimpleCycles(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);
int getNumberOfNonIsoCycles(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);

#endif
