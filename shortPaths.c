#include <string.h>
#include "graph.h"
#include "shortPaths.h"

char recursivePath(struct Vertex* v, struct VertexList* e) {
	if (e == NULL) {
		return 1;
	}

	if (strcmp(e->startPoint->label, v->label) == 0) {
		struct VertexList* neighbor;
		v->visited = 1;
		for (neighbor=v->neighborhood; neighbor!=NULL; neighbor=neighbor->next) {
			if (strcmp(e->label, neighbor->label) == 0) {
				char found = recursivePath(neighbor->endPoint, e->next);
				if (found) {
					return found;
				}		
			}
		}
		v->visited = 0;
	}
	
	/* if v->label != e->startPoint->label OR the remaining path can not be mapped to something below v */
	return 0;	
}

/**
Check if a graph g contains a certain  path of length at least 1.
If so, this function returns 1, otherwise 0.

The path needs to be given as a shallow graph containing edges. 
The labels of edges and vertices are compared. Accordingly,
the endpoint pointers of each edge must point to some existing vertex.

NULL values in the label pointers will probably result in strange errors.

Cheers!
**/
char containsPath(struct Graph* g, struct ShallowGraph* path) {
	int v;
	for (v=0; v<g->n; ++v) {
		char found = recursivePath(g->vertices[v], path->edges);
		if (found) {
			return found;
		}
	}
	return 0;
}