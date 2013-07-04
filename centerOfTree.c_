#include <stdio.h>
#include "graph.h"
#include "centerOfTree.h"


/**
 * Sets the ->lowPoint values of all vertices in a tree to 0.
 */
void setLowPoints(struct Vertex* v, struct Vertex* parent) {
	struct VertexList* idx;

	v->lowPoint = 0;
	for (idx=v->neighborhood; idx; idx = idx->next) {
		if (idx->endPoint != parent) {
			setLowPoints(idx->endPoint, v);
		}
	}
}


/**
 * Check if the vertex has degree 1 or 0 in O(1).
 * TODO refactor -> graph.h
 */
char isLeaf(struct Vertex* v) {
	struct VertexList* idx;
	int delta = 0;

	for (idx=v->neighborhood; idx; idx=idx->next) {
		if (delta < 2) {
			++delta;
		} else {
			break;
		}
	}
	return (delta <= 1);
}


/**
 * Tests, if v is a leaf or an isolated vertex according to the current shelling step.
 * If it has at most one neighbor with a lower shelling number, this method returns true.
 * sadly O(\delta(v)), as all neighbors (but one) could have a lower shelling number.
 *
 * TODO Theoretic question: Look at stars. Can one do better by checking stuff differently?
 */
char isShellingLeaf(struct Vertex* v) {
	struct VertexList* idx;
	int delta = 0;

	for (idx=v->neighborhood; idx; idx=idx->next) {
		if (delta < 2) {
			if (idx->endPoint->lowPoint == 0) {
				++delta;
			}
		} else {
			break;
		}
	}
	return (delta <= 1);
}


/**
 * Returns a list of all degree 1 vertices in the connected component of v.
 * leaves should point to an initially empty ShallowGraph.
 * This is a DFS.
 */
void getLeaves(struct Vertex* v, struct Vertex* p, struct ShallowGraph* leaves, struct ListPool* lp) {
	struct VertexList* idx;

	if (isLeaf(v)) {
		struct VertexList* e = getVertexList(lp);
		e->endPoint = v;
		appendEdge(leaves, e);
	}

	for (idx=v->neighborhood; idx; idx=idx->next) {
		if (idx->endPoint != p) {
			getLeaves(idx->endPoint, v, leaves, lp);
		}
	}
}

/*void getLeaves(struct Vertex* v, struct Vertex* parent, int maxDepth, struct ShallowGraph* leaves, struct ListPool* lp) {
	struct VertexList* idx;
	struct VertexList* e;

	/ * if v is a leaf, add it to leaves and return * /
	if (isLeaf(v, maxDepth)) {
		e = getVertexList(lp);
		e->endPoint = v;
		appendEdge(leaves, e);
	}

	/ * continue recursively * /
	for (idx=v->neighborhood; idx; idx=idx->next) {
		if ((idx->endPoint != parent) / *&& (idx->endPoint->visited <= maxDepth)* /) {
			getLeaves(idx->endPoint, v, maxDepth, leaves, lp);
		}
	}

} */

/**
 * TODO remove or merge with printVertexList
 */
void printList(struct ShallowGraph* currentLeaves) {
	struct VertexList* idx;

	printf("leaves: ");
	for (idx=currentLeaves->edges; idx; idx=idx->next) {
		printf("%i ", idx->endPoint->number);
	}
	printf("\n");
}

/**
 * This function returns the center of the tree containing the vertex v.
 * The center is either a single vertex or two vertices that are incident.
 *
 * The center of a tree is useful to define a canonical string for trees.
 * Instead of computing the canonical string of a tree rooted at v for each
 * vertex v in a tree, one only needs to compute the canonical string for the
 * tree(s) rooted at the center.
 *
 * The algorithm runs in O(n) and uses the ->lowPoint elements which will be
 * initialized to 0.
 *
 * TODO: This does not work atm.
 * maxDepth is used in the same way as in treeKernels.c to specify a subtree where
 * 0 < w->visited <= maxDepth for all vertices w.
 */
struct ShallowGraph* getTreeCenter(struct Vertex* v, int maxDepth, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* currentLeaves = getShallowGraph(sgp);
	struct ShallowGraph* newLeaves = getShallowGraph(sgp);
	struct VertexList* idx;
	int i;

	/* init lowPoints TODO necessary?*/
	setLowPoints(v, v);

	getLeaves(v, v, currentLeaves, sgp->listPool);

	/* shell the tree by iteratively burning leaves.
	 * Man, my mind has a strange kind of imagination... */
	for (i=1; 1; ++i) {

		/* if new currentLeaves contains one vertex, this is the center.
		 * if it contains two leaves, they are the center, iff they are
		 * connected */
		if (currentLeaves->m == 1) {
			break;
		}
		if (currentLeaves->m == 2) {
			if (isIncident(currentLeaves->edges->endPoint, currentLeaves->lastEdge->endPoint)) {
				break;
			}
			if (currentLeaves->edges->endPoint == currentLeaves->lastEdge->endPoint) {
				dumpVertexList(sgp->listPool, popEdge(currentLeaves));
				break;
			}
		}

		/* burn the current leaves */
		for (idx=currentLeaves->edges; idx; idx=idx->next) {
			struct VertexList* e;

			/* set shelling depth of current vertex */
			idx->endPoint->lowPoint = i;

			/* find the !unique! neighbor of idx->endPoint, that has lowPoint 0,
			 * if it is a leaf in the tree where all vertices have lowPoint 0. */
			for (e=idx->endPoint->neighborhood; e; e=e->next) {
				if (e->endPoint->lowPoint == 0) {

					/* If so, add it to the newLeaves list */
					if (isShellingLeaf(e->endPoint)) {
						struct VertexList* f = getVertexList(sgp->listPool);
						f->endPoint = e->endPoint;
						appendEdge(newLeaves, f);
					}

					/* ...and stop*/
					break;
				}
			}
		}

		/* currentLeaves are no longer current */
		dumpShallowGraph(sgp, currentLeaves);
		currentLeaves = newLeaves;

		/* initialize newLeaves for next while iteration */
		newLeaves = getShallowGraph(sgp);
	}

	/* garbage collection */
	dumpShallowGraph(sgp, newLeaves);

	/* currentLeaves contains the center of the tree */
	return currentLeaves;
}
