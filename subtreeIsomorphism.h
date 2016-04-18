#ifndef SUBTREE_ISOMORPHISM_H_
#define SUBTREE_ISOMORPHISM_H_

#include "cachedGraph.h"

// int*** createCube(int x, int y);
// void freeCube(int*** cube, int x, int y);
void dumpCube();

int*** createCube(int x, int y);

char subtreeCheck(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char subtreeCheckF(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char subtreeCheckFF(struct Graph* g, struct Graph* h, struct GraphPool* gp);
char subtreeCheckCached(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct CachedGraph* cacheB);
char subtreeCheck3(struct Graph* g, struct Graph* h, struct GraphPool* gp);

#endif /* SUBTREE_ISOMORPHISM_H_ */
