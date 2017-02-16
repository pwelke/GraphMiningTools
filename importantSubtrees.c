
#include <stddef.h>
#include <malloc.h>
#include <assert.h>

#include "graph.h"
#include "connectedComponents.h"
#include "iterativeSubtreeIsomorphism.h"
#include "importantSubtrees.h"


/**
 * Return a list of graphs corresponding to the connected components of g (or NULL, if g is empty).
 */
struct Graph* graph2Components(struct Graph* g, struct GraphPool* gp) {

	// init visited to 0
	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->lowPoint = -1;
	}
	// mark vertices of connected components in g
	int nComponents = 0;
	for (int v=0; v<g->n; ++v) {
		if (g->vertices[v]->lowPoint == -1) {
			markComp(g->vertices[v], nComponents);
			++nComponents;
		}
	}

	struct Graph* result = NULL;
	if (nComponents > 0) {
		// create graph struct for each found component
		struct Graph** components = malloc(nComponents * sizeof(struct Graph*));
		for (int i=0; i<nComponents; ++i) {
			components[i] = getGraph(gp);
		}
		// make list from array.
		for (int i=0; i<nComponents-1; ++i) {
			components[i]->next = components[i+1];
		}
		// count number of vertices in each component
		for (int v=0; v<g->n; ++v) {
			int c = g->vertices[v]->lowPoint;
			++(components[c]->n);
		}
		// create vertex arrays for graphs
		for (int i=0; i<nComponents; ++i) {
			setVertexNumber(components[i], components[i]->n);
		}
		// store vertices of g in the corresponding component graphs
		for (int v=0; v<g->n; ++v) {
			int c = g->vertices[v]->lowPoint;
			int newpos = components[c]->number;
			components[c]->vertices[newpos] = g->vertices[v];
			g->vertices[v]->number = newpos;
			components[c]->number++;
		}
		// TODO set components->m, components->number, ...
		result = components[0];
		free(components);
	}
	return result;
}

void graph2ComponentCleanup(struct Graph* g, struct Graph* components, struct GraphPool* gp) {
	for (struct Graph* c=components; c!=NULL; c=c->next) {
		for (int v=0; v<c->n; ++v) {
			c->vertices[v] = NULL;
		}
		c->n = 0;
	}
	dumpGraphList(gp, components);
	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->number = v;
	}
}

int importanceCount(struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	struct Graph* components = graph2Components(g, gp);
	int freq = 0;
	for (struct Graph* component=components; component!=NULL; component=component->next) {
		if (isSubtree(component, h, gp)) {
			++freq;
		}
	}
	graph2ComponentCleanup(g, components, gp);
	return freq;
}

double importanceRelative(struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	struct Graph* components = graph2Components(g, gp);
	int freq = 0;
	int count = 0;
	for (struct Graph* component=components; component!=NULL; component=component->next) {
		if (isSubtree(component, h, gp)) {
			++freq;
		}
		++count;
	}
	graph2ComponentCleanup(g, components, gp);
	return ((double)freq) / ((double)count);
}

char isImportantSubtreeAbsolute(struct Graph* g, struct Graph* h, int absoluteThreshold, struct GraphPool* gp) {
	int importance = importanceCount(g, h, gp);
	return importance >= absoluteThreshold;
}

char isImportantSubtreeRelative(struct Graph* g, struct Graph* h, double relativeThreshold, struct GraphPool* gp) {
	double importance = importanceRelative(g, h, gp);
	return importance >= relativeThreshold;
}

/*
 * Gap Amplification a la locality sensitive hashing. OR_4(AND_4)
 */
char andorEmbedding(struct Graph* g, struct Graph* h, struct GraphPool* gp) {
	struct Graph* components = graph2Components(g, gp);
	char match = 0;
	int freq = 0;
	int componentCount = 0;
	for (struct Graph* component=components; component!=NULL; component=component->next) {
		if (isSubtree(component, h, gp)) {
			++freq;
		}
		++componentCount;
		if (componentCount % 4 == 0) {
			if (freq == 4) {
				match = 1;
			} else {
				freq = 0;
			}
		}
	}
	assert(componentCount == 16);
	graph2ComponentCleanup(g, components, gp);
	return match;
}
