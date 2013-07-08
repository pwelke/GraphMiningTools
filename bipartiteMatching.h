#ifndef BIPARTITE_MATCHING_H_
#define BIPARTITE_MATCHING_H_

int bipartiteMatchingFastAndDirty(struct Graph* g, struct GraphPool* gp);
struct ShallowGraph* getMatching(struct Graph* g, struct ShallowGraphPool* sgp);
struct ShallowGraph* bipartiteMatching(struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif /* BIPARTITE_MATCHING_H_ */