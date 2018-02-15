#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "graph.h"
#include "graphPrinting.h"

void printGraphAidsFormat(struct Graph* g, FILE* out) {
	int i;
	// print header line
	fprintf(out, "# %i %i %i %i\n", g->number, g->activity, g->n, g->m);

	// print vertex labels
	for (i=0; i<g->n; ++i) {
		fprintf(out, "%s ", g->vertices[i]->label);
	}
	fputc('\n', out);

	// print edges
	for (i=0; i<g->n; ++i) {
		struct VertexList* e;
		for (e=g->vertices[i]->neighborhood; e!=NULL; e=e->next) {
			if (e->startPoint->number < e->endPoint->number) {
				fprintf(out, "%i %i %s ", e->startPoint->number + 1, e->endPoint->number + 1, e->label);
			}
		}
	}
	fputc('\n', out);
}


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
	fflush(stdout);
}

/**
 * Print some information about a ShallowGraph
 */
void printLabelledShallowGraph(struct ShallowGraph* g) {
	
	struct ShallowGraph* index = g;
	int i = 0;
	do {
		if (index) {
			struct VertexList* e;
			printf("Graph %i has %i edges:\n", i, index->m);
			for (e=index->edges; e!=NULL; e=e->next) {
				printf("(%s)%s(%s)\n",e->startPoint->label, e->label, e->endPoint->label);
			}
			printf("\n");
			index = index->next;
			++i;
		} else {
			/* if index is NULL, the input pointed to a list and not to a cycle */
			break;
		}
	} while (index != g);
	fflush(stdout);
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

	if (!silent) {
		printf("Result contains %i structures\n", i);
		fflush(stdout);
	}
	
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
			printf("Graph %i has id %i, %i vertices, and %i edges:\n", i, index->number, index->n, index->m);
			for (j=0; j<index->n; ++j) {
				if (index->vertices[j]) {
					printf("Vertex %i has label %s and the following neighbors:\n", index->vertices[j]->number, index->vertices[j]->label);
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
	fflush(stdout);
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
	fflush(stdout);
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
	fflush(stdout);
}

/**
 * Print some information about a single edge to the screen
 */
void printEdge(struct VertexList *e) {
	printf("(%i, %i)", e->startPoint->number, e->endPoint->number);
	printf(" %s\n", e->label);
	fflush(stdout);
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
	fflush(stdout);
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
			printf("%s", s1);
			printf("\n");
			printf("%s", s2);
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
	fflush(stdout);
	return diffs;
}


void printDirectedGraphDotFormat(struct Graph* g, FILE* out) {
	fprintf(out, "digraph %i {\n", g->number);
	for (int v=0; v<g->n; ++v) {
		fprintf(out, "%i [label=%s];\n", v, g->vertices[v]->label);
	}
	for (int v=0; v<g->n; ++v) {
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			fprintf(out, "%i -> %i [label=%s];\n", e->startPoint->number, e->endPoint->number, g->vertices[v]->label);
		}
	}
	fprintf(out, "}\n");
}

void printGraphDotFormat(struct Graph* g, FILE* out) {
	fprintf(out, "graph %i {\n", g->number);
	for (int v=0; v<g->n; ++v) {
		fprintf(out, "%i [label=%s];\n", v, g->vertices[v]->label);
	}
	for (int v=0; v<g->n; ++v) {
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->startPoint->number < e->endPoint->number) {
				fprintf(out, "%i -- %i [label=%s];\n", e->startPoint->number, e->endPoint->number, g->vertices[v]->label);
			}
		}
	}
	fprintf(out, "}\n");
}
