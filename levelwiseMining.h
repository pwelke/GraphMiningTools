#ifndef LEVELWISE_MINING_H_
#define LEVELWISE_MINING_H_

void getVertexAndEdgeHistograms(char* fileName, int minGraph, int maxGraph, struct Vertex* frequentVertices, struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct Graph* refinementGraph(struct Graph* g, int currentVertex, struct VertexList* newEdge, struct GraphPool* gp);
struct Graph* extendPattern(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp);
struct Vertex* generateCandidateSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* edgeSearchTree2ShallowGraph(struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void freeFrequentEdgeShallowGraph(struct GraphPool* gp, struct ShallowGraphPool* sgp, struct ShallowGraph* edges);

#endif
