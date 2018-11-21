#ifndef TREEENUMERATION_H_
#define TREEENUMERATION_H_	

#include "graph.h"
#include "intSet.h"

struct Graph* extendRootedPatternAllWays(struct Graph* g, struct ShallowGraph* candidateEdges, struct GraphPool* gp);
struct IntSet* aprioriCheckExtensionRootedReturnList(struct Graph* extension, struct Vertex* lowerLevel, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif
