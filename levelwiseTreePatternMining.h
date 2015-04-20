#ifndef LEVELWISE_TREE_PATTERN_MINING_H_
#define LEVELWISE_TREE_PATTERN_MINING_H_

#include "stdio.h"
#include "graph.h"

// struct TreeDB
// {
// 	/* a list of graphs being the spanning trees of the graph with number n */
// 	struct Graph* treeSet;
// 	/* TreeDB is a list */
// 	struct TreeDB* next;
// 	/* unique identifier of the graph this treeSet corresponds to */
// 	int number;
// 	/* for pruning */
// 	char boring;
// };

int getVertexAndEdgeHistograms(char* fileName, struct Vertex* frequentVertices, struct Vertex* frequentEdges, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp);
// void dumpTreeDB(struct GraphPool* gp, struct TreeDB* db);

struct ShallowGraph* edgeSearchTree2ShallowGraph(struct Vertex* frequentEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void freeFrequentEdgeShallowGraph(struct GraphPool* gp, struct ShallowGraphPool* sgp, struct ShallowGraph* edges);
int makeGraphsAndPointers(struct Vertex* root, struct Vertex* current, struct Graph** patterns, struct Vertex** pointers, int i, struct ShallowGraph* prefix, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void scanDBNoCache(char* fileName, struct Vertex* currentLevel, struct Graph** refinements, 
					struct Vertex** pointers, int n, int threshold, int nGraphs,
					double fraction, FILE* keyValueStream, struct GraphPool* gp, struct ShallowGraphPool* sgp,
					int (*embeddingOperator)(struct ShallowGraph*, struct Graph**, double, int, int, int**, struct Vertex**, struct GraphPool*));

int checkIfSubIso(struct ShallowGraph* transactionTrees, struct Graph** patternTrees, 
					int i, int n, int** features, struct Vertex** pointers, struct GraphPool* gp);
int checkIfImportantSubIso(struct ShallowGraph* transactionTrees, struct Graph** patternTrees, double fraction, 
					int i, int n, int** features, struct Vertex** pointers, struct GraphPool* gp);
int checkIfSubIsoCompatible(struct ShallowGraph* transactionTrees, struct Graph** patternTrees, double fraction, 
					int i, int n, int** features, struct Vertex** pointers, struct GraphPool* gp);

#endif
