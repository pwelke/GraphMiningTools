#ifndef HP_CACTUS_H_
#define HP_CACTUS_H_

char isTraceableCactus(struct Graph* g, struct ShallowGraphPool* sgp);
char isThisCactusTraceable(struct Graph* g, struct ShallowGraphPool* sgp);

char isWeaklyTraceableUnsafe(struct Graph* g, struct ShallowGraphPool* sgp);
char isWeaklyTraceable(struct Graph* g, struct ShallowGraphPool* sgp);

#endif