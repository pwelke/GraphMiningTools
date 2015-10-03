#ifndef WEISFEILER_LEHMAN_H_
#define WEISFEILER_LEHMAN_H_ 

#include "graph.h"

char* getWLLabel(struct Vertex* v, struct Vertex* trie, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct Graph* weisfeilerLehmanRelabel(struct Graph* g, struct Vertex* wlLabels, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif