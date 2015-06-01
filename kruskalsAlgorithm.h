#ifndef KRUSKALS_ALGORITHM_H_
#define KRUSKALS_ALGORITHM_H_

struct ShallowGraph* kruskalMST(struct Graph* g, struct VertexList** sortedEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif