#ifndef LIST_COMPONENTS_H_
#define LIST_COMPONENTS_H_

#include "graph.h"

void markConnectedComponents(struct Vertex *v, int component);

struct ShallowGraph* findBiconnectedComponents(struct Graph* g, struct ShallowGraphPool* gp);
struct Graph* partitionIntoForestAndCycles(struct ShallowGraph* list, struct Graph* original, struct GraphPool* p, struct ShallowGraphPool* gp);


#endif /* LIST_COMPONENTS_H_ */
