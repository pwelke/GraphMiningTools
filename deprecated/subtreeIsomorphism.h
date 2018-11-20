#ifndef SUBTREE_ISOMORPHISM_H_
#define SUBTREE_ISOMORPHISM_H_

#include "cachedGraph.h"

void dumpCube();
char subtreeCheck3(struct Graph* g, struct Graph* h, struct GraphPool* gp);
struct Vertex* subtreeCheckRooted(struct Graph* g, int gRoot, struct Graph* h, int hRoot, struct GraphPool* gp);

#endif /* SUBTREE_ISOMORPHISM_H_ */
