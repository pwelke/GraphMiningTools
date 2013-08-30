#include <stddef.h>
#include <malloc.h>

#include "graph.h"
#include "cachedGraph.h"

/**
 * Allocates an array of n pointers to vertices and sets the number of vertices of
 * g to n. The array is initialized to NULL.
 */
struct Vertex** __setVertexNumber(struct Graph* g, int n, struct GraphPool* gp) {
	int i;
	
	if (n<0) {
		printf("Error allocating memory for array of negative size %i\n", n);
		return NULL;
	}
	
	g->n = n;
	g->vertices = malloc(n * sizeof(struct Vertex*));

	for (i=0; i<n; ++i) {
		g->vertices[i] = getVertex(gp->vertexPool);
	}

	return g->vertices;
}

void __free(struct Vertex** vertices, int maxCap, struct GraphPool* gp) {
	int i;

	for (i=0; i<maxCap; ++i) {
		dumpVertex(gp->vertexPool, vertices[i]);
		vertices[i] = NULL;
	}
	free(vertices);
}

struct CachedGraph* initCachedGraph(struct GraphPool* gp, int size) {
	struct CachedGraph* cache;
	if ((cache = malloc(sizeof(struct CachedGraph)))) {
		cache->gp = gp;
		cache->inUse = 0;
		cache->g = getGraph(gp);
		__setVertexNumber(cache->g, size, gp);
		cache->maxCap = size;
		cache->next = NULL;
	}
	return cache;
}


struct Graph* getCachedGraph(int size, struct CachedGraph* cache) {
	 if (!cache->inUse) {
	 	cache->inUse = 1;
		if (cache->maxCap < size) {
			__free(cache->g->vertices, cache->maxCap, cache->gp);
			__setVertexNumber(cache->g, size, cache->gp);
			cache->maxCap = size;
		} else {
			cache->g->n = size;	
		}
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
		cache->g->vertices[w]->neighborhood = NULL;
		cache->g->vertices[w]->d = 0;
		cache->g->vertices[w]->lowPoint = 0;
		cache->g->vertices[w]->visited = 0;
	}
	cache->inUse = 0;
}


void dumpCachedGraph(struct CachedGraph* cache) {
	cache->g->n = cache->maxCap;
	dumpGraph(cache->gp, cache->g);
	free(cache);
}
