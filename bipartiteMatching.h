#ifndef BIPARTITE_MATCHING_H_
#define BIPARTITE_MATCHING_H_

int bipartiteMatchingFastAndDirty(struct Graph* g, struct GraphPool* gp);
struct ShallowGraph* getMatching(struct Graph* g, struct ShallowGraphPool* sgp);
struct ShallowGraph* bipartiteMatching(struct Graph* h, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void addResidualEdges(struct Vertex* v, struct Vertex* w, struct ListPool* lp);
void initBipartite(struct Graph* B);
char augment(struct Vertex* s, struct Vertex* t);
void removeSandT(struct Graph* B, struct Vertex* s, struct Vertex* t, struct GraphPool* gp);
void setFlag(struct VertexList* e, int flag);

struct Vertex* getMatchedVertex(struct Vertex* a);
char isMatched(struct Vertex* a);

#endif /* BIPARTITE_MATCHING_H_ */