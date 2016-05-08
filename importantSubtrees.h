#ifndef IMPORTANT_SUBTREES_H_
#define IMPORTANT_SUBTREES_H_

#include "graph.h"

int importanceCount(struct Graph* g, struct Graph* h, struct GraphPool* gp);
double importanceRelative(struct Graph* g, struct Graph* h, struct GraphPool* gp);
char isImportantSubtreeAbsolute(struct Graph* g, struct Graph* h, int absoluteThreshold, struct GraphPool* gp);
char isImportantSubtreeRelative(struct Graph* g, struct Graph* h, double relativeThreshold, struct GraphPool* gp);

#endif
