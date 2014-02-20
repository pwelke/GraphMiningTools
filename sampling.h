#ifndef SAMPLING_H_
#define SAMPLING_H_

struct VertexList* randomSuccessor(struct Vertex* root);

struct ShallowGraph* sampleSpanningTree(struct Graph* g, struct ShallowGraphPool* sgp);
struct ShallowGraph* sampleSpanningTree2(struct Graph* g, int root, struct ShallowGraphPool* sgp);

struct VertexList** sampleSpanningEdges(struct Graph* g, int root);

#endif