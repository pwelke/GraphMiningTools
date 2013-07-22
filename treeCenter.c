#include <malloc.h>
#include <limits.h>

#include "graph.h"
#include "canonicalString.h"
#include "treeCenter.h"

/**
After calling this method for some vertex in a tree with value
0, the ->lowpoint's of all vertices in a tree hold the distance 
of that vertex to the root and the ->used's of edges hold the
depth of the branch dangling at that edge (always relative to the 
initial root).
*/
int markEdges(struct Vertex* root, int value) {
	struct VertexList* e;
	int max = value;

	root->lowPoint = value;

	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->lowPoint > value) {
			/* markEdges returns the depth of the subtree rooted at 
			e->endPoint */
			int current = markEdges(e->endPoint, value + 1);
			e->used = current;
			if (current > max) {
				max = current;
			}
		}
	}
	/* return the maximum depth of any subtree rooted at root */
	return max;
}

/**
To find the center of a tree, we have to implicitly find the 
longest path in the tree. 
Think of it like this: You are at root and know the depths of 
all branches dangling at root. Consider the two largest depths.
If they are equal, we are in the middle of the longest path.
If they differ by one, the longest path has odd length and there 
are two center vertices. 
If the difference is larger, we move the root to the child that is
the root of the largest branch. 
(We then have to update the depths to fit to the new root)
THe algorithm writes the number(s) of the found center(s) to 
centers.
*/
void moveCenter(struct Vertex* root, struct Vertex* parent, int value, int* centers) {
	struct VertexList* e;
	int max = INT_MIN;
	struct VertexList* max1 = NULL;
	struct VertexList* max2 = NULL;
	struct VertexList* parentEdge = NULL;

	/* update depths to fit to the new root */
	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint != parent) {
			e->used -= value;
		} else {
			parentEdge = e;
		}
	}

	/* find maximum depth branch */
	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->used > max) {
			max = e->used;
			max1 = e;
		}
	}

	/* find second maximum depth branch */
	max = INT_MIN;
	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e != max1) {
			if (e->used > max) {
				max = e->used;
				max2 = e;
			}
		}
	}

	/* if the current node is no leaf */
	if (max2 != NULL) {

		/* if the two deepest branches have same depth, we are at the 
		center */
		if (max1->used == max2->used) {
			centers[0] = 2;
			centers[1] = root->number;
			return;
		}

		/* this is the case where the longest path has odd length
		and we get two center nodes */
		if (max1->used == max2->used + 1) {
			centers[0] = 3;
			centers[1] = root->number;
			centers[2] = max1->endPoint->number;
			return;
		}
		/* ...and the symmetric case */
		if (max2->used == max1->used + 1) {
			centers[0] = 3;
			centers[1] = root->number;
			centers[2] = max2->endPoint->number;
			return;
		}
	}
	/* if we are not in the first step, we have to increase 
	the depth of the branch we came from */
	if (parentEdge != NULL) {
		parentEdge->used = max2->used + 1;
	}
	/* move the center to the root of the deepest branch */
	moveCenter(max1->endPoint, root, value + 1, centers);
}


/**
Find the center vertex or center vertices of the given tree.
The algorithm works by choosing a root and eventually moving 
int to the center of the tree step by step. Runtime is O(n).
->lowPoint's of vertices in tree and ->used's of VertexLists 
are used (algorithm inits them itself).
The tree may not be empty.

The returned value is a pointer to an array containing the
numbers of the center nodes. 
*/
int* treeCenter(struct Graph* tree) {
	int* centers = malloc(3 * sizeof(int));
	int i;

	if (tree->n < 3) {
		/* if the tree consists of one or two vertices, the
		center is clear (and the root-rebase algorithm does not work)
		*/
		if (tree->n == 1) {
			centers[0] = 2;
			centers[1] = 0;
		} else {
			centers[0] = 3;
			centers[1] = 0;
			centers[2] = 1;
		} /* zero vertex trees are not allowed */
	} else {
		/* init lowpoints and do the root-rebase algorithm
		starting from the first vertex we can get a hold of */
		for (i=0; i<tree->n; ++i) {
			tree->vertices[i]->lowPoint = INT_MAX;
		}
		markEdges(tree->vertices[0], 0);
		moveCenter(tree->vertices[0], NULL, 0, centers);
	}
	return centers;
}


/**
Compute a canonical string of a (free/unrooted) tree by
choosing its center nodes to be the roots for the canonical
string representation. If there are two center nodes, the 
lexicographically smaller of the two canonical strings is 
returned.
Runtime is thus O(n)
*/
struct ShallowGraph* treeCenterCanonicalString(struct Graph* tree, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* cString;
	int* center = treeCenter(tree);

	cString = canonicalStringOfRootedTree(tree->vertices[center[1]], tree->vertices[center[1]], sgp);
	
	if (center[0] == 3) {
		struct ShallowGraph* cString2 = canonicalStringOfRootedTree(tree->vertices[center[2]], tree->vertices[center[2]], sgp);
		if (lexicographicComparison(cString, cString2) < 0) {
			dumpShallowGraph(sgp, cString2);
		} else {
			dumpShallowGraph(sgp, cString);
			cString = cString2;
		}
	}
	free(center);
	return cString;
}
