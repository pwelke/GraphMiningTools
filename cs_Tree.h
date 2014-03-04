#ifndef CANONICALSTRING_H
#define CANONICALSTRING_H

#include "graph.h"

/* canonical strings for trees */
struct ShallowGraph* getTreePatterns(struct Graph* forest, struct ShallowGraphPool *sgp);
struct ShallowGraph* canonicalStringOfRootedTree(struct Vertex* vertex, struct Vertex* parent, struct ShallowGraphPool *p);
struct ShallowGraph* canonicalStringOfRootedLevelTree(struct Vertex* vertex, struct Vertex* parent, int maxDepth, struct ShallowGraphPool *p);
struct ShallowGraph* canonicalStringOfLevelTree(struct ShallowGraph* vertexList, int maxDepth, struct ShallowGraphPool* sgp);

struct Graph* treeCanonicalString2Graph(struct ShallowGraph* pattern, struct GraphPool* gp);
void treeCanonicalString2ExistingGraph(struct ShallowGraph* pattern, struct Graph* g, struct GraphPool* gp);

#endif /* CANONICALSTRING_H */
