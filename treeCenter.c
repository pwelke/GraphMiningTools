#include <stdlib.h>
#include <limits.h>

#include "cs_Compare.h"
#include "cs_Tree.h"
#include "vertexQueue.h"
#include "treeCenter.h"

/**
After calling this method for some vertex in a tree with value
0, the ->lowpoint's of all vertices in a tree hold the distance 
of that vertex to the root and the ->used's of edges hold the
depth of the branch dangling at that edge (always relative to the 
initial root).
*/
int __markEdges(struct Vertex* root, int value) {
	struct VertexList* e;
	int max = value;

	root->lowPoint = value;

	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint->lowPoint > value) {
			/* __markEdges returns the depth of the subtree rooted at 
			e->endPoint */
			int current = __markEdges(e->endPoint, value + 1);
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
void __moveCenter(struct Vertex* root, struct Vertex* parent, int value, int depth, int* centers) {
	struct VertexList* e;
	int max = INT_MIN;
	struct VertexList* max1 = NULL;
	struct VertexList* max2 = NULL;

	/* update depths to fit to the new root */
	for (e=root->neighborhood; e!=NULL; e=e->next) {
		if (e->endPoint != parent) {
			e->used -= depth;
		} else {
			e->used = value;
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

		/* move the center to the root of the deepest branch */
		__moveCenter(max1->endPoint, root, max2->used + 1, depth + 1, centers);
	} else {
		/* if we start at a leaf */
		__moveCenter(max1->endPoint, root, 1, 1, centers);
	}	
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
		__markEdges(tree->vertices[0], 0);
		__moveCenter(tree->vertices[0], tree->vertices[0] /*NULL*/, 0, 0, centers);	
	}
	return centers;
}


/**
 * Compute the distances of the vertices in tree to center and store them in ->lowPoints.
 * The algorithm uses (and initializes) ->lowPoint's of vertices in tree and ->used's of
 * VertexLists. Its runtime is O(n).
 * It returns the largest distance to the center, i.e.,
 * \[ \max_{v \in V(G)} \min_{c \in C(G)} d(v, c) \]
 * where C(G) is the set of center vertices of the input tree g.
 */
int computeDistanceToCenter(struct Graph* g, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* queue = getShallowGraph(sgp);

	// copmute tree center
	int* center = treeCenter(g);

	// clean the ->lowpoints
	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->lowPoint = -1;
	}

	// init the queue with (bi)center
	for (int i=1; i<center[0]; ++i) {
		addToVertexQueue(g->vertices[center[i]], queue, sgp);
		g->vertices[center[i]]->lowPoint = 0;
	}
	free(center);

	// do bfs, mark vertices according to their distance to the (bi)center
	int depth = 0;
	for (struct Vertex* v=popFromVertexQueue(queue, sgp); v!=NULL; v=popFromVertexQueue(queue, sgp)) {
		depth = v->lowPoint + 1;
		for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
			if (e->endPoint->lowPoint == -1) {
				e->endPoint->lowPoint = depth;
				addToVertexQueue(e->endPoint, queue, sgp);
			}
		}
	}
	dumpShallowGraph(sgp, queue);

	// depth is set too high by the last loop over the leaves in the outer shell
	return depth - 1;
}
