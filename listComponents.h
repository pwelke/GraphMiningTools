#ifndef LIST_COMPONENTS_H_
#define LIST_COMPONENTS_H_

#include "graph.h"


void markConnectedComponent(struct Vertex *v, int component);
int getAndMarkConnectedComponents(struct Graph* g);
struct Graph* listConnectedComponents(struct Graph* g, struct GraphPool* gp);

int getNumberOfBridges(struct Graph* g, struct ShallowGraphPool* sgp);
int getNumberOfBridgeTrees(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);
int getNumberOfBlocks(struct Graph* g, struct ShallowGraphPool* sgp);

int* computeCycleDegrees(struct ShallowGraph* biconnectedComponents, int n);
int* computeCriticality(struct ShallowGraph* biconnectedComponents, int n);

int getMaxCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp);
int getMinCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp);

struct Graph* partitionIntoForestAndCycles(struct ShallowGraph* list, struct Graph* original, struct GraphPool* p, struct ShallowGraphPool* gp);
struct ShallowGraph* listBiconnectedComponents(struct Graph* g, struct ShallowGraphPool* gp);


#endif /* LIST_COMPONENTS_H_ */
