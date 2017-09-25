#ifndef LIST_SPANNING_TREES_H_
#define LIST_SPANNING_TREES_H_

#include "graph.h"

struct ShallowGraph* listSpanningTrees(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);
struct ShallowGraph* listKSpanningTrees(struct Graph* original, int* k, struct ShallowGraphPool* sgp, struct GraphPool* gp);
long int countSpanningTrees(struct Graph* g, long int maxBound, struct ShallowGraphPool* sgp, struct GraphPool* gp);
int countNonisomorphicSpanningTrees(struct Graph* g, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif /* LIST_SPANNING_TREES_H_ */
