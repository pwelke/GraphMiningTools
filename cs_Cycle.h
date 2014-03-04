#ifndef CS_CYCLE_H_
#define CS_CYCLE_H_

/* canonical strings for cycles */ 
struct ShallowGraph* permutateCycle(struct ShallowGraph* g);
struct ShallowGraph* getCanonicalStringOfCycle(struct ShallowGraph* cycle, struct ShallowGraphPool* sgp);
struct ShallowGraph* getCyclePatterns(struct ShallowGraph* cycles, struct ShallowGraphPool* sgp);

#endif
