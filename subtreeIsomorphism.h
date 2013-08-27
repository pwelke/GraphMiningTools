#ifndef SUBTREE_ISOMORPHISM_H_
#define SUBTREE_ISOMORPHISM_H_

char subtreeCheck(struct Graph* g, struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);

int*** createCube(int x, int y);
void freeCube(int*** cube, int x, int y);

int* findLeaves(struct Graph* g, int root);
struct ShallowGraph* removeVertexFromBipartiteInstance(struct Graph* B, int v, struct ShallowGraphPool* sgp);
void addVertexToBipartiteInstance(struct ShallowGraph* temp);

#endif /* SUBTREE_ISOMORPHISM_H_ */