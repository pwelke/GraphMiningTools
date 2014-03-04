#ifndef CS_COMPARE_H_
#define CS_COMPARE_H_

#include "graph.h"

int compareDirectedEdges(const void* p1, const void* p2);

int compareVertexLists(const struct VertexList* e1, const struct VertexList* e2);
int compareCanonicalStrings(const struct ShallowGraph *g1, const struct ShallowGraph *g2);
int lexCompCS(const void* e1, const void* e2);

#endif
