#ifndef CS_OUTERPLANAR_H_
#define CS_OUTERPLANAR_H_

#include "graph.h"

/* canonical strings for outerplanar graphs */
struct ShallowGraph* canonicalStringOfOuterplanarBlock(struct ShallowGraph* hamiltonianCycle, struct ShallowGraph* diagonals, struct ShallowGraphPool* sgp);
struct ShallowGraph* canonicalStringOfOuterplanarGraph(struct ShallowGraph* original, struct ShallowGraphPool* sgp, struct GraphPool* gp);

struct ShallowGraph* __getCycleAndDiagonals(struct Graph* g, struct ShallowGraphPool* sgp);

#endif
