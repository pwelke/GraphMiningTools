#ifndef CS_OUTERPLANAR_H_
#define CS_OUTERPLANAR_H_

/* canonical strings for outerplanar graphs */
struct ShallowGraph* getCanonicalStringOfOuterplanarBlock(struct ShallowGraph* hamiltonianCycle, struct ShallowGraph* diagonals, struct ShallowGraphPool* sgp);
struct ShallowGraph* getOuterplanarCanonicalString(struct ShallowGraph* original, struct ShallowGraphPool* sgp, struct GraphPool* gp);

#endif