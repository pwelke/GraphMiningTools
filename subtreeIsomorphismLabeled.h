#ifndef SUBTREE_ISOMORPHISM_H_
#define SUBTREE_ISOMORPHISM_H_

char subtreeCheck(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char subtreeCheckF(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char subtreeCheckFF(struct Graph* g, struct Graph* h, struct GraphPool* gp);
char subtreeCheckCached(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct CachedGraph* cacheB);

#endif /* SUBTREE_ISOMORPHISM_H_ */