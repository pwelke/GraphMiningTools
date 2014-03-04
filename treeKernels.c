#include <stdio.h>
#include "graph.h"
#include "cs_Tree.h"
#include "treeKernels.h"


/**
 * Restores ->visited = 0 after calling edgeSort
 */
void cleanUpEdgeSort(struct Vertex* v, struct ShallowGraphPool* sgp) {
	struct VertexList* e;
	struct VertexList* f;
	struct VertexList* first = getVertexList(sgp->listPool);
	struct ShallowGraph* fifo = getShallowGraph(sgp);

	/* create artificial edge to start bfs */
	first->endPoint = v;
	first->startPoint = v;
	appendEdge(fifo, first);

	/* bfs */
	while((e = popEdge(fifo))) {


			/* store distance */
			e->endPoint->visited = 0;

			/* put children in fifo queue */
			for (f=e->endPoint->neighborhood; f; f = f->next) {
				if (f->endPoint->visited) {
					appendEdge(fifo, shallowCopyEdge(f, sgp->listPool));
				}
			}

			dumpVertexList(sgp->listPool, e);

	}

	dumpShallowGraph(sgp, fifo);
}

/**
 * returns a list of vertices sorted in the order a dfs from v traverses them.
 * for each e in the list, e->endPoint points to the vertex and e->endPoint->visited
 * denotes the distance of that vertex to v.
 */
struct ShallowGraph* edgeSort(struct Vertex* v, int maxDepth, struct ShallowGraphPool* sgp) {
	struct VertexList* e;
	struct VertexList* f;
	struct VertexList* first = getVertexList(sgp->listPool);
	struct ShallowGraph* output = getShallowGraph(sgp);
	struct ShallowGraph* fifo = getShallowGraph(sgp);

	/* create artificial edge to start bfs */
	first->endPoint = v;
	first->startPoint = v;
	appendEdge(fifo, first);

	/* bfs */
	while((e = popEdge(fifo))) {

		/* breaking condition */
		if (e->startPoint->visited > maxDepth) {
			dumpVertexList(sgp->listPool, e);
			break;
		}

		/* store distance */
		e->endPoint->visited = e->startPoint->visited + 1;
		appendEdge(output, e);

		/* put children in fifo queue */
		for (f=e->endPoint->neighborhood; f; f = f->next) {
			if (f->endPoint->visited == 0) {
				appendEdge(fifo, shallowCopyEdge(f, sgp->listPool));
			}
		}
	}

	dumpShallowGraph(sgp, fifo);

	return output;
}


/**
 * This is the method described as BFSSubtreeEnumeration in the documentation.
 * It returns a list of canonical strings of rooted trees that are obtained
 * by doing bfs's of depth 0 up to maxDepth on each vertex in t
 */
struct ShallowGraph* bfsSubtreeEnumeration(struct Graph* t, int maxDepth, struct ShallowGraphPool* sgp) {
	int i;
	struct ShallowGraph* results = NULL;

	for (i=0; i<t->n; ++i) {
		if (t->vertices[i]) {

			int j;
			struct ShallowGraph* sorted = edgeSort(t->vertices[i], maxDepth, sgp);

			/* if maxDepth is too large, the for loop below will return
			 * multiple copies of the largest level tree. */
			if (maxDepth > sorted->lastEdge->endPoint->visited) {
				maxDepth = sorted->lastEdge->endPoint->visited;
			}

			/* compute and store canonical string of the tree rooted at v for each level
			 * TODO This can change. Maybe compute canonical string for free tree... */
			for (j=1; j<=maxDepth; ++j) {
				struct ShallowGraph* level = canonicalStringOfRootedLevelTree(t->vertices[i], NULL, j, sgp);
				level->next = results;
				results = level;
			}

			cleanUpEdgeSort(t->vertices[i], sgp);
			dumpShallowGraph(sgp, sorted);
		}
	}
	return results;
}

/**
 * A different approach to BFSSubtreeEnumeration. The rooted tree approach
 * produces too many patterns, thus this one tries to select free trees as patterns.
 *
 * It returns a list of canonical strings of trees that are obtained
 * by doing bfs's of depth 0 up to maxDepth on each vertex in t
 */
struct ShallowGraph* bfsFreeSubtreeEnumeration(struct Graph* t, int maxDepth, struct ShallowGraphPool* sgp) {
	int i;
	struct ShallowGraph* results = NULL;

	for (i=0; i<t->n; ++i) {
		if (t->vertices[i]) {

			int j;
			struct ShallowGraph* sorted = edgeSort(t->vertices[i], maxDepth, sgp);

			/* if maxDepth is too large, the for loop below will return
			 * multiple copies of the largest level tree. */
			if (maxDepth > sorted->lastEdge->endPoint->visited) {
				maxDepth = sorted->lastEdge->endPoint->visited;
			}

			/* compute and store canonical string of the tree rooted at v for each level
			 * TODO This can change. Maybe compute canonical string for free tree... */
			for (j=1; j<=maxDepth; ++j) {
				struct ShallowGraph* level = canonicalStringOfLevelTree(sorted, j, sgp);
				level->next = results;
				results = level;
			}

			cleanUpEdgeSort(t->vertices[i], sgp);
			dumpShallowGraph(sgp, sorted);
		}
	}
	return results;
}



/**
 * List ALL subtrees of a tree recursively
 */
void recSubtreeEnum(struct Graph* current, struct ShallowGraph* forbiddenEdges, struct ShallowGraph* allEdges, int* number, struct ShallowGraphPool* sgp) {
	struct VertexList* e;
	struct VertexList* startOfForbidden = forbiddenEdges->edges;

	for (e=allEdges->edges; e; e=e->next) {

		/* check if e is in forbiddenEdges */
		struct VertexList* idx;
		char allowed = 1;
		for (idx=forbiddenEdges->edges; idx; idx=idx->next) {
			/* if edge is forbidden */
			if (((e->startPoint == idx->startPoint) && (e->endPoint == idx->endPoint))
					|| ((e->startPoint == idx->endPoint) && (e->endPoint == idx->startPoint))) {
				allowed = 0;
				break;
			}
		}

		/* if its not, list current + e and continue recursively */
		if (allowed) {

			int v = e->startPoint->number;
			int w = e->endPoint->number;

			/* if adding e produces a tree, list that tree and continue recursively
			 * this assumes, that t is a tree */
			if ((current->m == 0)
					|| ((current->vertices[v]->neighborhood != NULL) && (current->vertices[w]->neighborhood == NULL))
					|| ((current->vertices[w]->neighborhood != NULL) && (current->vertices[v]->neighborhood == NULL))) {
				struct VertexList* tmp;

				addEdge(current->vertices[v], shallowCopyEdge(e, sgp->listPool));
				addEdge(current->vertices[w], inverseEdge(e, sgp->listPool));

				++current->m;

				/* here, output is produced */
				++(*number);

				/* recursive call for (T + e, F) */
				recSubtreeEnum(current, forbiddenEdges, allEdges, number, sgp);

				/* remove edge from current, add it to forbidden list */
				tmp = current->vertices[v]->neighborhood;
				current->vertices[v]->neighborhood = tmp->next;
				dumpVertexList(sgp->listPool, tmp);

				tmp = current->vertices[w]->neighborhood;
				current->vertices[w]->neighborhood = tmp->next;
				dumpVertexList(sgp->listPool, tmp);

				--current->m;

				pushEdge(forbiddenEdges, shallowCopyEdge(e, sgp->listPool));

				recSubtreeEnum(current, forbiddenEdges, allEdges, number, sgp);

				break;

			}
		}
	}

	/* reallow all edges that were forbidden in the current recursion */
	for (e=forbiddenEdges->edges; e!=startOfForbidden; e=forbiddenEdges->edges) {
		dumpVertexList(sgp->listPool, popEdge(forbiddenEdges));
	}
}


/**
 * Enumerates all subtrees of a tree t
 * Note that the number of subtrees can be exponential in the number of nodes of t.
 * E.g. for stars it is 2^(n-1) + n - 1
 *
 * Thus the application of this method seems not reasoable for large trees,
 * where large means n>40
 */
int fullSubtreeEnumeration(struct Graph* t, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	int numberOfTrees = 0;

	/* get initially empty list of forbidden edges and E(t) */
	struct ShallowGraph* allEdges = getGraphEdges(t, sgp);
	struct ShallowGraph* forbiddenEdges = getShallowGraph(sgp);

	/* create $ current = (V(t), \emptyset) $ */
	struct Graph* current = getGraph(gp);
	int i;
	setVertexNumber(current, t->n);
	for (i=0; i<t->n; ++i) {
		current->vertices[i] = shallowCopyVertex(t->vertices[i], gp->vertexPool);
	}

	recSubtreeEnum(current, forbiddenEdges, allEdges, &numberOfTrees, sgp);


	/* garbage collection */
	dumpShallowGraph(sgp, allEdges);
	dumpShallowGraph(sgp, forbiddenEdges);
	dumpGraph(gp, current);

	return numberOfTrees;
}
