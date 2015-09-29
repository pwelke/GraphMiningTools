#ifndef TREEENUMERATION_H_
#define TREEENUMERATION_H_	

struct Graph* refinementGraph(struct Graph* g, int currentVertex, struct VertexList* newEdge, struct GraphPool* gp);
struct Graph* extendPattern(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp);
struct Vertex* generateCandidateTreeSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct Vertex* generateCandidateAprioriTreeSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct Vertex* generateCandidatePathSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif
