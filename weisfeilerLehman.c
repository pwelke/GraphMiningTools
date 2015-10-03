

#include <stdlib.h>
#include <string.h>
#include "loading.h"
#include "searchTree.h"
#include "weisfeilerLehman.h"

static int compare (const void * a, const void * b)
{
    return strcmp (*(char **) a, *(char **) b);
}

/**
Return a WL-label for vertex v.

trie should hold all WL-label strings that were computed for the current iteration so far and stores the new int labels
for compression. */
char* getWLLabel(struct Vertex* v, struct Vertex* trie, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	char* resultingLabel;
	struct Vertex* local = getVertex(gp->vertexPool);
	struct ShallowGraph* string = getShallowGraph(sgp);
	struct ShallowGraph* copy;
	int deg = degree(v);
	char** neighborlabels = malloc(deg * sizeof(char*));
	int i = 0;
	struct VertexList* e;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		neighborlabels[i] = e->endPoint->label;
		++i;
	}
	qsort(neighborlabels, deg, sizeof(char*), &compare);

	e = getVertexList(sgp->listPool);
	e->label = v->label;
	appendEdge(string, e);
	for (i=0; i<deg; ++i) {
		e = getVertexList(sgp->listPool);
		e->label = neighborlabels[i];
		appendEdge(string, e);
	}
	// fprintf(stderr, "adding string ");
	// printCanonicalString(string, stdout);
	copy = cloneShallowGraph(string, sgp);
	// fprintf(stderr, "copy          ");
	// printCanonicalString(copy, stdout);
	addToSearchTree(local, copy, gp, sgp);
	mergeSearchTrees(trie, local, 1, NULL, 0, trie, 0, gp);
	i = getID(trie, string);
	// fprintf(stderr, "resulting label %i\n", i);
	resultingLabel = intLabel(i);
	dumpShallowGraphCycle(sgp, string);
	free(neighborlabels);
	return resultingLabel;
}

/**
Do a single iteration of Weisfeiler-Lehman relabeling. Return a new graph with new labels.

I.e., the new label of each vertex will represent the label of the vertex and the label of each
neighbor of the vertex. These labels are compressed to integers for performance reasons.
*/
struct Graph* weisfeilerLehmanRelabel(struct Graph* g, struct Vertex* wlLabels, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* h = cloneGraph(g, gp);
	int v;

	for (v=0; v<g->n; ++v) {
		h->vertices[v]->label = getWLLabel(g->vertices[v], wlLabels, gp, sgp);
		h->vertices[v]->isStringMaster = 1;
	}
	// fprintf(stderr, "\n\n\n");
	// printStringsInSearchTree(wlLabels, stderr, sgp);
	return h;
}