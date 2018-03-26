#ifndef TREEENUMERATION_H_
#define TREEENUMERATION_H_	

#include "graph.h"
#include "intSet.h"

struct Graph* basicFilter(struct Graph* extension, struct Vertex* listOfGraphs, struct GraphPool* gp, struct ShallowGraphPool* sgp);

struct Graph* refinementGraph(struct Graph* g, int currentVertex, struct VertexList* newEdge, struct GraphPool* gp);
struct Graph* extendPatternAllWays(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp);
struct Graph* extendPatternOnLeaves(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp);
struct Graph* extendPatternOnOuterShells(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp, struct ShallowGraphPool* sgp);

struct IntSet* aprioriCheckExtensionReturnList(struct Graph* extension, struct Vertex* lowerLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif
