#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "graph.h"
#include "graphPrinting.h"


/**
 * Print some information about a ShallowGraph
 */
void printShallowGraph(struct ShallowGraph* g) {
	
	struct ShallowGraph* index = g;
	int i = 0;
	do {
		if (index) {
			printf("Graph %i has %i edges:\n", i, index->m);
			printVertexList(index->edges);
			printf("\n");
			index = index->next;
			++i;
		} else {
			/* if index is NULL, the input pointed to a list and not to a cycle */
			break;
		}
	} while (index != g);
	
}


/**
 * returns the number of ShallowGraphs in the list or cycle of ShallowGraphs
 */
int printShallowGraphCount(struct ShallowGraph* g, char silent) {

	struct ShallowGraph* index = g;
	int i = 0;
	do {
		if (index) {
			index = index->next;
			++i;
		} else {
			/* if index is NULL, the input pointed to a list and not to a cycle */
			break;
		}
	} while (index != g);

	if (!silent)
		printf("Result contains %i structures\n", i);

	return i;

}


/**
 * Print some information about a graph to the screen
 */
void printGraph(struct Graph* g) {
	
	struct Graph* index = g;
	int i= 0,j;

	do {
		if (index) {
			printf("Graph %i has %i edges:\n", i, index->m);
			for (j=0; j<index->n; ++j) {
				if (index->vertices[j]) {
					printf("Neighbors of vertex %i:\n", index->vertices[j]->number);
					printVertexList(index->vertices[j]->neighborhood);
				} else {
					printf("Vertex %i is not used by the current (induced) graph\n", j);
				}
				
			}
			printf("\n");
			index = index->next;
			++i;
		} else {
			/* if index is NULL, the input pointed to a list and not to a cycle */
			break;
		}
	} while (index != g);
	
}


/**
 * Print information about the edges contained in a graph to the screen
 */
void printGraphEdges(struct Graph *g) {
	int i;
	printf("Graph has %i vertices and %i edges:\n", g->n, g->m);
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {
			struct VertexList *idx;
			for (idx = g->vertices[i]->neighborhood; idx; idx = idx->next) {
				if (idx->endPoint->number > i) {
					printf("(%i, %i) %s d: (%i, %i) visited: (%i,  %i)\n", idx->startPoint->number, idx->endPoint->number, idx->label, idx->startPoint->d, idx->endPoint->d, idx->startPoint->visited, idx->endPoint->visited);
				}
			}
		}
	}
}

/**
 * Print some information about a single edge,
 * and all edges reachable from it, to the screen
 */
void printVertexList(struct VertexList *f) {
	struct VertexList* e;
	for (e=f; e; e=e->next) {
		printf("(%i, %i)", e->startPoint->number, e->endPoint->number);
		fflush(stdout);
		printf(" %s\n", e->label);
	}
}


/**
 * Print information about the edges contained in a graph to the screen
 */
void printGraphEdgesOfTwoGraphs(char* name, struct Graph *g, struct Graph* h) {
	int i;
	printf("%s 1 has %i vertices and %i edges:\n", name, g->n, g->m);
	printf("%s 2 has %i vertices and %i edges:\n", name, h->n, h->m);
	for (i=0; i<g->n; ++i) {
		printf("%i: ", i);
		if (g->vertices[i]) {
			struct VertexList *idx;
			for (idx = g->vertices[i]->neighborhood; idx; idx = idx->next) {
				printf("%i ", idx->endPoint->number);	
			}
		}
		printf("\n%i: ", i);
		if (h->vertices[i]) {
			struct VertexList *idx;
			for (idx = h->vertices[i]->neighborhood; idx; idx = idx->next) {
				printf("%i ", idx->endPoint->number);	
			}
		}
		printf("\n\n");
	}
}


void cleanString(char* s, int n) {
	int i;
	for (i=0; i<n; ++i) {
		s[i] = 0;
	}
}

char diffGraphs(char* name, struct Graph *g, struct Graph* h) {
	int n = 300;
	int diffs = 0;
	char* s1 = malloc(n * sizeof(char));
	char* s2 = malloc(n * sizeof(char));
	int i;
	cleanString(s1,n);
	cleanString(s2,n);

	sprintf(s1, "%s has %i vertices and %i edges:\n", name, g->n, g->m);
	sprintf(s2, "%s has %i vertices and %i edges:\n", name, h->n, h->m);

	if (strcmp(s1, s2) != 0) {
		++diffs;
	}
	for (i=0; i<g->n; ++i) {
		int store1 = 0;
		int store2 = 0;
		cleanString(s1, n);
		cleanString(s2, n);
		store1 += sprintf(s1, "%i: ", i);
		if (g->vertices[i]) {
			struct VertexList *idx;
			for (idx = g->vertices[i]->neighborhood; idx; idx = idx->next) {
				store1 += sprintf(s1+store1, "%i ", idx->endPoint->number);	
			}
		}
			store2 += sprintf(s2, "%i: ", i);
		if (h->vertices[i]) {
			struct VertexList *idx;
			for (idx = h->vertices[i]->neighborhood; idx; idx = idx->next) {
				store2 += sprintf(s2+store2, "%i ", idx->endPoint->number);	
			}
		}
		if (strcmp(s1, s2) != 0) {
			printf(s1);
			printf("\n");
			printf(s2);
			printf("\n\n");
			++diffs;
		}
	}
	free(s1);
	free(s2);

	if (diffs > 0) {
		printf("%s has %i vertices and %i edges:\n", name, g->n, g->m);
		printf("%s has %i vertices and %i edges:\n", name, h->n, h->m);
	}
	return diffs;
}