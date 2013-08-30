#ifndef LEVELWISE_MINING_H_
#define LEVELWISE_MINING_H_

struct TreeDB
{
	/* a list of graphs being the spanning trees of the graph with number n */
	struct Graph* treeSet;
	/* TreeDB is a list */
	struct TreeDB* next;
	/* unique identifier of the graph this treeSet corresponds to */
	int number;
	/* for pruning */
	char boring;
};

void getVertexAndEdgeHistograms(char* fileName, int minGraph, int maxGraph, struct Vertex* frequentVertices, struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void getVertexAndEdgeHistogramsP(char* fileName, int minGraph, int maxGraph, struct Vertex* frequentVertices, struct Vertex* frequentEdges, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void dumpTreeDB(struct GraphPool* gp, struct TreeDB* db);

struct Graph* refinementGraph(struct Graph* g, int currentVertex, struct VertexList* newEdge, struct GraphPool* gp);
struct Graph* extendPattern(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp);
struct Vertex* generateCandidateSet(struct Vertex* lowerLevel, struct ShallowGraph* extensionEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* edgeSearchTree2ShallowGraph(struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void freeFrequentEdgeShallowGraph(struct GraphPool* gp, struct ShallowGraphPool* sgp, struct ShallowGraph* edges);
int makeGraphsAndPointers(struct Vertex* root, struct Vertex* current, struct Graph** patterns, struct Vertex** pointers, int i, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void scanDB(char* fileName, struct Vertex* currentLevel, struct Graph** refinements, struct Vertex** pointers, int n, int minGraph, int maxGraph, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp);
#endif
