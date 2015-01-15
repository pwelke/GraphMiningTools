#ifndef UNSORTED_FILTERS_H_
#define UNSORTED_FILTERS_H_

char isTree(struct Graph* g);
char isCactus(struct Graph* g, struct ShallowGraphPool* sgp);
char isOuterplanarGraph(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);

int* computeCycleDegrees(struct Graph* g, struct ShallowGraphPool* sgp);

int getNumberOfBridges(struct Graph* g, struct ShallowGraphPool* sgp);
int getNumberOfBridgeTrees(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);
int getNumberOfBlocks(struct Graph* g, struct ShallowGraphPool* sgp);
int getMaxDegree(struct Graph* g);
int getMinDegree(struct Graph* g);
int getMaxCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp);
int getMinCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp);
int getNumberOfSimpleCycles(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);
int getNumberOfNonIsoCycles(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);

#endif