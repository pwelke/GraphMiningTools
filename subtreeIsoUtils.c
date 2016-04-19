/*
 * subtreeIsoUtils.c
 *
 *  Created on: Apr 18, 2016
 *      Author: pascal
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "subtreeIsoUtils.h"

/**
Wrapper for strcmp that can deal with NULL strings.
We fix the semantic of a vertex or edge label that is NULL as follows:
It matches every label and is matched by every label.
I.e. labelCmp always returns 0 if one of the arguments is NULL.
*/
int labelCmp(const char* l1, const char* l2) {
	return ((l1 == NULL) || (l2 == NULL)) ? 0 : strcmp(l1, l2);
}


/** dfs function local to getPostorder() */
static int dfs(struct Vertex* v, int value) {
	struct VertexList* e;

	/* to make this method save for graphs that are not trees */
	v->visited = -2;

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->visited == -1) {
			value = 1 + dfs(e->endPoint, value);
			e->endPoint->lowPoint = v->number;
		}
	}
	v->visited = value;
	return value;
}


/**
Compute a dfs order or postorder on g starting at vertex root.
The ->visited members of vertices are set to the position they have in the order
(starting with 0). Vertices that cannot be reached from root get ->visited = -1
The method returns an array of length g->n where position i contains the vertex number
of the ith vertex in the order.
The ->lowPoint s of vertices in g point to their parents in the postorder.
*/
int* getPostorder(struct Graph* g, int root) {
	int* order = malloc(g->n * sizeof(int));
	for (int i=0; i<g->n; ++i) {
		g->vertices[i]->visited = -1;
		order[i] = -1;
	}
	dfs(g->vertices[root], 0);
	g->vertices[root]->lowPoint = -1;
	for (int i=0; i<g->n; ++i) {
		if (g->vertices[i]->visited != -1) {
			order[g->vertices[i]->visited] = i;
		} else {
			/* should never happen if g is a tree */
			fprintf(stderr, "Vertex %i was not visited by dfs.\nThis can not happen, if g is a tree.\n", i);
		}
	}
	return order;
}


/**
Find all vertices reachable by augmenting paths that start with a non-matching edge
*/
void markReachable(struct Vertex* a, int num) {
	struct VertexList* e;

	a->visited = 1;
	// if (a->number < num)
	// printf("Mark %i visited\n", a->lowPoint);

	for (e=a->neighborhood; e!=NULL; e=e->next) {
		// printf("check edge (%i %i) flag %i\n", e->startPoint->lowPoint, e->endPoint->lowPoint, e->flag);
		if ((e->flag == 0) && (e->endPoint->visited == 0)) {
			markReachable(e->endPoint, num);
		}
	}
}
