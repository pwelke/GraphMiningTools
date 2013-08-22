#ifndef SUBTREE_ISOMORPHISM_H_
#define SUBTREE_ISOMORPHISM_H_

char subtreeCheck(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);

int*** createCube(int x, int y);
void freeCube(int*** cube, int x, int y);

#endif /* SUBTREE_ISOMORPHISM_H_ */