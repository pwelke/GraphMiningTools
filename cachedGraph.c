#include <stddef.h>
#include <malloc.h>

#include "graph.h"
#include "cachedGraph.h"
#include "intMath.h"


struct CachedGraph* initCachedGraph(struct GraphPool* gp, int size) {
	struct CachedGraph* cache;
	if ((cache = malloc(sizeof(struct CachedGraph)))) {
		cache->gp = gp;
		cache->inUse = 0;
		cache->g = createGraph(size, gp);
		// __setVertexNumber(cache->g, size, gp);
		cache->maxCap = size;
		cache->next = NULL;
	}
	return cache;
}


struct Graph* getCachedGraph(int size, struct CachedGraph* cache) {
	 if (!cache->inUse) {
	 	cache->inUse = 1;
		if (cache->maxCap < size) {
			// do the 'increase size by factor of two' thing to avoid many reallocs
			int newCap = max(size, 2 * cache->maxCap);
			
			// dump old graph
			cache->g->n = cache->maxCap;
			dumpGraph(cache->gp, cache->g);
			
			// create larger new graph
			cache->g = createGraph(newCap, cache->gp);
			cache->maxCap = newCap;
		} 
		cache->g->n = size;	
		return cache->g;
	} else {
		fprintf(stderr, "Graph is in use\n");
		return NULL;
	}
}

/**
 * "Return a cached graph to the cache".
 * That is, remove all edges and wipe its vertices, such that it can be used as if it were a newly created graph.
 *
 * This method is extremely hot in the localEasy* embedding operators, hence we try to avoid multiple dereferenceings of the same pointers.
 */
void returnCachedGraph(struct CachedGraph* cache) {
	struct Vertex** vertices = cache->g->vertices;
	struct ListPool* lp = cache->gp->listPool;
	int n = cache->g->n;
	for (int wi=0; wi<n; ++wi) {
		struct Vertex* w = vertices[wi];
		dumpVertexListRecursively(lp, w->neighborhood);
		wipeVertexButKeepNumber(w);
	}
	cache->inUse = 0;
}


void dumpCachedGraph(struct CachedGraph* cache) {
	cache->g->n = cache->maxCap;
	dumpGraph(cache->gp, cache->g);
	free(cache);
}
