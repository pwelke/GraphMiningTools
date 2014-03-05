#ifndef LIST_CYCLES_H_
#define LIST_CYCLES_H_

#include "graph.h"

struct ShallowGraph* readTarjanListAllCycles(struct Graph *g, struct ShallowGraphPool *sgp);
struct ShallowGraph* backtrack(struct Graph* g, struct Vertex* v, struct Vertex* parent, struct Vertex* s, int allowance, struct ShallowGraph* currentPath, struct ShallowGraphPool* sgp);
char DFS(struct Vertex* v, struct Vertex* parent, struct Vertex* target, int allowance);
char findPath(struct Vertex* v, struct Vertex* parent, struct Vertex* target, int allowance, struct ShallowGraph* path, struct ShallowGraphPool *sgp);


#endif
