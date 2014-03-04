#ifndef CS_COMPARE_H_
#define CS_COMPARE_H_

#include "graph.h"

int directedEdgeComparator(const void* p1, const void* p2);

int lexicographicComparison(const struct ShallowGraph *g1, const struct ShallowGraph *g2);
int lexComp(const void* e1, const void* e2);
int compareVertexLists(const struct VertexList* e1, const struct VertexList* e2);

#endif
