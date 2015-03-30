#ifndef SUBTREE_ISOMORPHISM_H_
#define SUBTREE_ISOMORPHISM_H_

#include "cachedGraph.h"

// int*** createCube(int x, int y);
// void freeCube(int*** cube, int x, int y);
void dumpCube();

// int* findLeaves(struct Graph* g, int root);
// struct ShallowGraph* removeVertexFromBipartiteInstance(struct Graph* B, int v, struct ShallowGraphPool* sgp);
// void addVertexToBipartiteInstance(struct ShallowGraph* temp);

char subtreeCheck(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char subtreeCheckF(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char subtreeCheckFF(struct Graph* g, struct Graph* h, struct GraphPool* gp);
char subtreeCheckCached(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct CachedGraph* cacheB);

#endif /* SUBTREE_ISOMORPHISM_H_ */