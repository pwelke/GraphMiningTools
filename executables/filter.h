#ifndef FILTER_H_
#define FILTER_H_

char isTree(struct Graph* g);

int getNumberOfBridges(struct Graph* g, struct ShallowGraphPool* sgp);
int getNumberOfBlocks(struct Graph* g, struct ShallowGraphPool* sgp);

#endif