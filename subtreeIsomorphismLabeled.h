#ifndef SUBTREE_ISOMORPHISM_H_
#define SUBTREE_ISOMORPHISM_H_

char subtreeCheckL(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char subtreeCheckLF(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
char subtreeCheckLFF(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif /* SUBTREE_ISOMORPHISM_H_ */