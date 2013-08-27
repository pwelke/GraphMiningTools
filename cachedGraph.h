#ifndef CACHED_GRAPH_H_
#define CACHED_GRAPH_H_

/**
struct CachedGraph holds a graph with preallocated vertex array and 
its maximum size. For situations, where a single graph needs to be
created over and over again, it may be useful (in terms of speed) to
not free and reallocate the vertex array, as it is done when calling
dumpGraph()
*/
struct CachedGraph {
	char inUse;
	struct Graph* g;
	int maxCap;
	struct CachedGraph* next;
	struct GraphPool* gp;
};

struct CachedGraph* initCachedGraph(struct GraphPool* gp, int size);
struct Graph* getCachedGraph(int size, struct CachedGraph* cache);
void returnCachedGraph(struct CachedGraph* cache);
void dumpCachedGraph(struct CachedGraph* cache);


#endif