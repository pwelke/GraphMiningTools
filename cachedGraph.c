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


void returnCachedGraph(struct CachedGraph* cache) {
	int w;
	for (w=0; w<cache->g->n; ++w) {
		dumpVertexListRecursively(cache->gp->listPool, cache->g->vertices[w]->neighborhood);
		wipeVertex(cache->g->vertices[w]);
		cache->g->vertices[w]->number = w;
	}
	cache->inUse = 0;
}


void dumpCachedGraph(struct CachedGraph* cache) {
	cache->g->n = cache->maxCap;
	dumpGraph(cache->gp, cache->g);
	free(cache);
}
