
#include <stddef.h>
#include <malloc.h>

#include "graph.h"
#include "connectedComponents.h"
#include "iterativeSubtreeIsomorphism.h"



struct Graph* graph2Components(struct Graph* g, struct GraphPool* gp) {
	// init visited to 0
	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->visited = 0;
	}
	// mark vertices of connected components in g
	int nComponents = 1;
	for (int v=0; v<g->n; ++v) {
		if (g->vertices[v]->visited == 0) {
			markComp(g->vertices[v], nComponents);
			++nComponents;
		}
	}
	// create graph struct for each found component
	struct Graph** components = malloc(nComponents * sizeof(struct Graph*));
	for (int i=0; i<nComponents; ++i) {
		components[i] = getGraph(gp);
		if (i<nComponents-1) {
			components[i]->next = components[i+1];
		}
	}
	// count number of vertices in each component
	for (int v=0; v<g->n; ++v) {
		int c = g->vertices[v]->visited - 1;
		++components[c]->n;
	}
	// create vertex arrays for graphs
	for (int i=0; i<nComponents; ++i) {
		setVertexNumber(components[i], components[i]->n);
	}
	// store vertices of g in the corresponding component graphs
	for (int v=0; v<g->n; ++v) {
		int c = g->vertices[v]->visited - 1;
		int newpos = components[c]->number;
		components[c]->vertices[newpos] = g->vertices[v];
		g->vertices[v]->number = newpos;
		components[c]->number++;
	}
	// TODO set components->m, components->number, ...
	struct Graph* result = components[0];
	free(components);

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


