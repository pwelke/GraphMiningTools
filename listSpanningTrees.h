#ifndef LIST_SPANNING_TREES_H_
#define LIST_SPANNING_TREES_H_

struct ShallowGraph* listSpanningTrees(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);
long int countSpanningTrees(struct Graph* g, long int maxBound, struct ShallowGraphPool* sgp, struct GraphPool* gp);

#endif /* LIST_SPANNING_TREES_H_ */
