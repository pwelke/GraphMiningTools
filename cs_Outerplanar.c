#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "intMath.h"
#include "cs_Parsing.h"
#include "cs_Compare.h"
#include "cs_Cycle.h"
#include "outerplanar.h"
#include "cs_Outerplanar.h"

//debug
#include "graphPrinting.h"

/*****************************************************************************************
 ************************************ Outerplanar Blocks *********************************
 *****************************************************************************************/


/**
 * Comparator that compared two directed edges wrt the lexicographic order of (flag, used)
 */
int __compareDiagonals(const void* e1, const void* e2) {

	struct VertexList* e = *((struct VertexList**)e1);
	struct VertexList* f = *((struct VertexList**)e2);

	if (e->flag - f->flag == 0) {
		return e->used - f->used;
	} else {
		return e->flag - f->flag;
	}
}


/**
 * !Be careful with this method! See compareCycles in cs_Cycle.c for more details
 *
 * Compare two cycles of equal length. Caution, the input parameters point to the edges
 * BEFORE the edges starting the cycle.
 * This is due to the usage of this method in permutateCycle().
 */
struct VertexList* __compareBlockRepresentations(struct VertexList* c1, struct VertexList* c2, int m, int o1, int o2) {
	struct VertexList* idx1 = c1;
	struct VertexList* idx2 = c2;


	/* check for differences in the following edges */
	for (idx1=idx1->next, idx2=idx2->next;
			(idx1!=c1);
			idx1=idx1->next, idx2=idx2->next) {

		/* check for differences in the edge label */
		if (strcmp(idx1->label, idx2->label) < 0) {
			return c1;
		}
		if (strcmp(idx1->label, idx2->label) > 0) {
			return c2;
		}
		if (strcmp(idx1->label, idx2->label) == 0) {
			/* if edge labels do not differ, check for differences in endPoint label */
			if (strcmp(idx1->endPoint->label, idx2->endPoint->label) < 0) {
				return c1;
			}
			if (strcmp(idx1->endPoint->label, idx2->endPoint->label) > 0) {
				return c2;
			}
		}
	}

	/* check for differences in the edge label of the first edge*/
	if (strcmp(idx1->label, idx2->label) < 0) {
		return c1;
	}
	if (strcmp(idx1->label, idx2->label) > 0) {
		return c2;
	}
	if (strcmp(idx1->label, idx2->label) == 0) {
		/* if edge labels do not differ, check for differences in endPoint label */
		if (strcmp(idx1->endPoint->label, idx2->endPoint->label) < 0) {
			return c1;
		}
		if (strcmp(idx1->endPoint->label, idx2->endPoint->label) > 0) {
			return c2;
		}
	}


	/* if no difference is found in the hamiltonian cycle, check the diagonals */
	for (idx1=idx1->next, idx2=idx2->next; idx1!=c1; idx1=idx1->next, idx2=idx2->next) {
		struct VertexList* e;
		int i;

		int neighbors1 = 0;
		int neighbors2 = 0;

		struct VertexList** narray1;
		struct VertexList** narray2;

		/* count number of diagonals that go from the current vertex to a vertex with a higher
		 * index wrt the current permutation */
		for (e=idx1->endPoint->neighborhood; e; e=e->next) {
			if (mod(e->startPoint->d - o1, m + 1) < mod(e->endPoint->d - o1, m + 1)) {
				++neighbors1;
			}
		}
		for (e=idx2->endPoint->neighborhood; e; e=e->next) {
			if (mod(e->startPoint->d - o2, m + 1) < mod(e->endPoint->d - o2, m + 1)) {
				++neighbors2;
			}
		}



		/* if there are more edges with a low start vertex index, the resulting cstring is smaller */
		if (neighbors1 < neighbors2) {
			return c2;
		}
		if (neighbors2 < neighbors1) {
			return c1;
		}

		/* special cases where current vertices have no or just one incident diagonal */
		if (neighbors1 == 0) {
			continue;
		}
		if (neighbors1 == 1) {
			if (mod(idx1->endPoint->neighborhood->endPoint->d - o1, m + 1) > mod(idx2->endPoint->neighborhood->endPoint->d - o2, m + 1)) {
				return c2;
			} else {
				return c1;
			}
		}

		/* otherwise we have to sort the edges wrt their endpoints modulo the permutation */

		/* sort the edges according to their cycle indices modulo the permutation */
		narray1 = malloc(neighbors1 * sizeof(struct VertexList*));
		narray2 = malloc(neighbors2 * sizeof(struct VertexList*));
		for (i=0, e=idx1->endPoint->neighborhood; i<neighbors1; e=e->next) {
			e->flag = mod(e->startPoint->d - o1, m + 1);
			e->used = mod(e->endPoint->d - o1, m + 1);
			if (e->flag < e->used) {
				narray1[i] = e;
				++i;
			}
		}
		for (i=0, e=idx2->endPoint->neighborhood; i<neighbors2; e=e->next) {
			e->flag = mod(e->startPoint->d - o2, m + 1);
			e->used = mod(e->endPoint->d - o2, m + 1);
			if (e->flag < e->used) {
				narray2[i] = e;
				++i;
			}
		}

		qsort(narray1, neighbors1, sizeof(struct VertexList*), __compareDiagonals);
		qsort(narray2, neighbors2, sizeof(struct VertexList*), __compareDiagonals);

		/* check if the indices of the endpoints of the diagonals differ */
		for (i=0; i<neighbors1; ++i) {
			if (__compareDiagonals(&(narray1[i]), &(narray2[i])) < 0) {
				free(narray1);
				free(narray2);
				return c1;
			}
			if (__compareDiagonals(&(narray1[i]), &(narray2[i])) > 0) {
				free(narray1);
				free(narray2);
				return c2;
			}
			/* otherwise continue with the next loop iteration */
		}
		free(narray1);
		free(narray2);

	}

	/* if neither checking the hamiltonian cycle nor checking the diagonals resulted
	 * in any difference, return c1 */
	return c1;

}


/**
 * This method creates the string representation of a diagonal in an outerplanar block.
 * It returns "( start end label )" where whitespaces stand for "next vertex list".
 */
void __appendDiagonal(struct ShallowGraph* diags, int start, int end, char* label, struct ListPool* p) {
	appendEdge(diags, getInitialisatorEdge(p));

	appendEdge(diags, getVertexList(p));
	diags->lastEdge->label = malloc(12*sizeof(char));
	diags->lastEdge->isStringMaster = 1;
	sprintf(diags->lastEdge->label, "%i", start);

	appendEdge(diags, getVertexList(p));
	diags->lastEdge->label = malloc(12*sizeof(char));
	diags->lastEdge->isStringMaster = 1;
	sprintf(diags->lastEdge->label, "%i", end);

	appendEdge(diags, getVertexList(p));
	diags->lastEdge->label = label;

	appendEdge(diags, getTerminatorEdge(p));
}


/**
 * Finds the cyclic permutation that results in a lexicographically smallest
 * representation of the input cycle, if one considers the canonical string beginning
 * with the edge label of the first edge in the ->edge list followed by the vertex label
 * of its endpoint etc.
 */
struct ShallowGraph* __permutateBlock(struct ShallowGraph* cycle, struct ShallowGraphPool* sgp) {
	struct VertexList* start = cycle->edges;
	struct VertexList* end = assertLastPointer(cycle);
	/* best points to the edge before the starting edge of the lex smallest cycle */
	struct VertexList* best = cycle->edges;
	int bestOffset = 2;
	/* return */
	struct ShallowGraph* cString;
	struct ShallowGraph* secondPart = getShallowGraph(sgp);
	/* indices */
	struct VertexList* idx;
	int i;

	/* make actual cycle */
	end->next = start;

	/* give each vertex a number according to its position in cycle */
	for (idx=start->next, i=1; idx!=start; idx=idx->next, ++i) {
		idx->startPoint->d = i;
	}
	start->endPoint->d = 0;

	/* offset=3 as in the first step the permutation starting at edge after the second vertex
	 * in cycle is compared to the permutation starting after the first */
	for (idx = start->next, i=3; idx != start; idx=idx->next, i=(i+1)%(cycle->m + 1)) {
		best = __compareBlockRepresentations(best, idx, cycle->m, bestOffset, i);
		if (best == idx) {
			bestOffset = i;
		}
	}

	/* turn cycle s.t. the shallow graph is lex. smallest */
	cycle->edges = best->next;
	cycle->lastEdge = best;
	best->next = NULL;

	/* compute canonical string of the component */
	for (idx=cycle->edges; idx; idx=idx->next) {
		struct VertexList* e;

		int neighbors = 0;

		struct VertexList** narray1;

		/* count number of diagonals that go from the current vertex to a vertex with a higher
		 * index wrt the current permutation */
		for (e=idx->endPoint->neighborhood; e; e=e->next) {
			if (mod(e->startPoint->d - bestOffset, cycle->m + 1) < mod(e->endPoint->d - bestOffset, cycle->m + 1)) {
				++neighbors;
			}
		}

		/* special cases where current vertices have no or just one incident diagonal */
		if (neighbors == 0) {
			continue; /* the idx loop */
		}

		if (neighbors == 1) {
			/* add edge representation to secondPart. There may be more than one incident diagonals, but just one that
			 * satisfies startpoint < endpoint. This is the edge we are searching. */
			for (e=idx->endPoint->neighborhood; e; e=e->next) {
				if (mod(e->startPoint->d - bestOffset, cycle->m + 1) < mod(e->endPoint->d - bestOffset, cycle->m + 1)) {
					__appendDiagonal(secondPart,
							mod(e->startPoint->d - bestOffset, cycle->m + 1),
							mod(e->endPoint->d - bestOffset, cycle->m + 1),
							e->label,
							sgp->listPool);

					break; /* inner for, there is just one edge */
				}
			}
			continue; /* the idx loop */
		}

		/* otherwise: sort the edges according to their cycle indices modulo the permutation */
		narray1 = malloc(neighbors * sizeof(struct VertexList*));

		for (i=0, e=idx->endPoint->neighborhood; i<neighbors; e=e->next) {
			e->flag = mod(e->startPoint->d - bestOffset, cycle->m + 1);
			e->used = mod(e->endPoint->d - bestOffset, cycle->m + 1);
			if (e->flag < e->used) {
				narray1[i] = e;
				++i;
			}
		}

		qsort(narray1, neighbors, sizeof(struct VertexList*), __compareDiagonals);

		/* append string representation of diagonal */
		for (i=0; i<neighbors; ++i) {
			__appendDiagonal(secondPart, narray1[i]->flag, narray1[i]->used, narray1[i]->label, sgp->listPool);
		}

		/* garbage collection */
		free(narray1);
	}


	cString = canonicalStringOfCycle(cycle, sgp);

	//debug
	printCanonicalString(cString, stdout);
	printCanonicalString(secondPart, stdout);

	cString->lastEdge->next = secondPart->edges;
	cString->lastEdge = secondPart->lastEdge;
	cString->m = cString->m + secondPart->m;
	pushEdge(cString, getInitialisatorEdge(sgp->listPool));
	appendEdge(cString, getTerminatorEdge(sgp->listPool));

	//debug
	fprintf(stdout, "something\n");
	printCanonicalString(cString, stdout);

	/* garbage collection */
	secondPart->edges = NULL;
	dumpShallowGraph(sgp, secondPart);

	return cString;
}


/**
 * Compute the canonical string of an outerplanar block given as hamiltonian cycle and list of diagonals.
 * This method assumes that graph, the start- and endpoints of the edges point to, contains no edges.
 *
 * If you want to use this method in a context, where the underlying graph is not empty, you either
 * have to delete all edges from it or you have to grate an empty new graph and change the start-
 * and endpoints of edges in hamiltonianCycle and diagonals accordingly.
 */
struct ShallowGraph* canonicalStringOfOuterplanarBlock(struct ShallowGraph* hamiltonianCycle, struct ShallowGraph* diagonals, struct ShallowGraphPool* sgp) {
	struct VertexList* idx;
	struct ShallowGraph* result1;
	struct ShallowGraph* result2;
	struct ShallowGraph* inverse = inverseCycle(hamiltonianCycle, sgp);

	/* special case. if block is a cycle, return the canonical string of that cycle */
	if (diagonals->m == 0) {
		dumpShallowGraph(sgp, diagonals);
		dumpShallowGraph(sgp, inverse);
		return getCyclePatterns(hamiltonianCycle, sgp);
	}

	/* add diagonals to the graph underlying the shallowgraphs */
	for (idx=diagonals->edges; idx; idx=idx->next) {
		struct VertexList* e = shallowCopyEdge(idx, sgp->listPool);
		e->next = idx->startPoint->neighborhood;
		idx->startPoint->neighborhood = e;

		e = inverseEdge(idx, sgp->listPool);
		e->next = idx->endPoint->neighborhood;
		idx->endPoint->neighborhood = e;
	}

	/* now find the permutation and orientation of the hamiltonian cycle that results in the lex.
	 * smallest string */

	result1 = __permutateBlock(hamiltonianCycle, sgp);

	//debug
	printCanonicalString(result1, stdout);

	result2 = __permutateBlock(inverse, sgp);

	//debug
	printCanonicalString(result2, stderr);

	/* garbage collection */
	dumpShallowGraph(sgp, inverse);
	dumpShallowGraph(sgp, hamiltonianCycle);
	dumpShallowGraph(sgp, diagonals);

	/* compare cStrings and return the better one */
	if (compareVertexLists(result1->edges, result2->edges) < 0) {
		dumpShallowGraph(sgp, result2);
		return result1;
	} else {
		dumpShallowGraph(sgp, result1);
		return result2;
	}

}




/**
 * Check if g is outerplanar using the algorithm
 * of Sarah Mitchell, except that instead of the linear
 * bucket sort, stdlibs qsort is used.
 *
 * This algorithm returns a ShallowGraph c containing the hamiltonian cycle of g.
 * c->next points to a ShallowGraph d containing the diagonals of g.
 * If g is not outerplanar, NULL is returned. 
 *
 * The algorithm alters g but does not dump the remainder.
 *
 * Mitchell, S. [1979]: Linear Algorithms to Recognize Outerplanar and
 * Maximal Outerplanar Graphs, Information Processing Letters Volume 9,
 * number 5, 16.12.1979
 */
struct ShallowGraph* __getCycleAndDiagonals(struct Graph* g, struct ShallowGraphPool* sgp) {
	/* data structures described by Mitchell[1979] */
	struct ShallowGraph* list = getShallowGraph(sgp);
	struct ShallowGraph* pairs = getShallowGraph(sgp);
	struct ShallowGraph* edges = getGraphEdges(g, sgp);
	struct VertexList* node = NULL;
	struct VertexList* pair = NULL;
	/* data structures for canonical string computation */
	struct ShallowGraph* hamiltonianCycle = getShallowGraph(sgp);
	struct ShallowGraph* diagonals = getShallowGraph(sgp);
	/* data structures for sorting */
	struct VertexList** edgeArray;
	struct VertexList** pairArray;
	/* indices */
	struct VertexList* idx;
	int i, j, k;
	int n = 0;
	char found;


	/*
	 * Get a list "list" of vertices of degree 2 and mark them as such in the graph
	 * by setting ->d = 1. Also, count the number of not-NULL vertices.
	 * This is useful if the algorithm gets some "induced subgraph" where some
	 * vertices are not used.
	 */
	for (i=0; i<g->n; ++i) {
		if (g->vertices[i]) {

			++n;

			/* if vertex has degree 2, add it to list and mark it as such */
			if (isDegreeTwoVertex(g->vertices[i])) {
				struct VertexList* e = getVertexList(sgp->listPool);
				e->endPoint = g->vertices[i];
				pushEdge(list, e);

				g->vertices[i]->d = 1;
			}
		}
	}

	/* first check. number of edges ok ? */
	if (g->m > 2 * n - 3) {
		/* printf("incorrect edge count\n"); */
		dumpShallowGraph(sgp, list);
		dumpShallowGraph(sgp, pairs);
		dumpShallowGraph(sgp, edges);
		dumpShallowGraph(sgp, hamiltonianCycle);
		dumpShallowGraph(sgp, diagonals);
		return NULL;
	}

	/* second check: enough vertices of degree 2? */
	if (list->m < 2) {
		/* printf("not enough vertices of degree 2\n"); */
		dumpShallowGraph(sgp, list);
		dumpShallowGraph(sgp, pairs);
		dumpShallowGraph(sgp, edges);
		dumpShallowGraph(sgp, hamiltonianCycle);
		dumpShallowGraph(sgp, diagonals);
		return NULL;
	}

	/*
	 * Successively remove vertices of degree 2
	 */
	for (i=1, node=list->edges; /*node && */  (i<=n-2);  ++i, node=node->next) {

		struct Vertex* v = node->endPoint;

		/* near and next are the two vertices adjacent to node */
		struct Vertex* near = v->neighborhood->endPoint;
		struct Vertex* next = v->neighborhood->next->endPoint;

		/* if the edges leading to near and next are artificial triangulation
		 * edges that were added below in a previous step, add them to the
		 * edges list. Do it s.t. startPoint->number <= endPoint->number, as
		 * there will be lexicographic sorting */
		if (v->neighborhood->flag) {
			if (v->number < near->number) {
				appendEdge(edges, shallowCopyEdge(v->neighborhood, sgp->listPool));
			} else {
				appendEdge(edges, inverseEdge(v->neighborhood, sgp->listPool));
			}
		}
		if (v->neighborhood->next->flag) {
			if (v->number < next->number) {
				appendEdge(edges, shallowCopyEdge(v->neighborhood->next, sgp->listPool));
			} else {
				appendEdge(edges, inverseEdge(v->neighborhood->next, sgp->listPool));
			}
		}

		/* canonical string: add the edges to hamiltonianCycle, if they are no triangulation
		 * edges (artificial or original!). Which is recorded in ->used */
		if (!v->neighborhood->used) {
			appendEdge(hamiltonianCycle, shallowCopyEdge(v->neighborhood, sgp->listPool));
		}
		if (!v->neighborhood->next->used) {
			appendEdge(hamiltonianCycle, shallowCopyEdge(v->neighborhood->next, sgp->listPool));
		}

		/* add the pair (near, next) to pairs list, sort lexicographically
		 we have to add pair to the adjacency list of the graph, if this edge
		 does not already exist. Mark newly added edges in the flag tag. */
		pair = getVertexList(sgp->listPool);

		if (near->number < next->number) {

			pair->startPoint = near;
			pair->endPoint = next;

			/* this is for the case where g is not maximal outerplanar and
			 * a "triangulation edge" is added */
			if (!isIncident(near, next)) {
				struct VertexList* tmp = shallowCopyEdge(pair, sgp->listPool);
				tmp->flag = 1;
				addEdge(near, tmp);

				tmp = inverseEdge(pair, sgp->listPool);
				tmp->flag = 1;
				addEdge(next, tmp);
			} else {
				/* it the edge is an original edge of g, it is a diagonal and will be stored in
				 * diagonals (if that has not already been done). */
				for (idx=near->neighborhood; idx; idx=idx->next) {
					if (idx->endPoint == next) {
						if (!idx->flag && !idx->used) {
							appendEdge(diagonals, shallowCopyEdge(idx, sgp->listPool));
						}
						break;
					}
				}
			}
		} else {

			pair->startPoint = next;
			pair->endPoint = near;

			/* this is for the case where g is not maximal outerplanar and
			 * a "triangulation edge" is added */
			if (!isIncident(near, next)) {
				struct VertexList* tmp = shallowCopyEdge(pair, sgp->listPool);
				tmp->flag = 1;
				addEdge(next, tmp);

				tmp =  inverseEdge(pair, sgp->listPool);
				tmp->flag = 1;
				addEdge(near, tmp);
			} else {
				/* it the edge is an original edge of g, it is a diagonal and will be stored in
				 * diagonals (if that has not already been done). */
				for (idx=near->neighborhood; idx; idx=idx->next) {
					if (idx->endPoint == next) {
						if (!idx->flag && !idx->used) {
							appendEdge(diagonals, shallowCopyEdge(idx, sgp->listPool));
						}
						break;
					}
				}
			}
		}

		pushEdge(pairs, pair);

		/* canonical string: mark the new edge as triangulation edge via ->used */
		for (idx=near->neighborhood; idx; idx=idx->next) {
			if (idx->endPoint == next) {
				idx->used = 1;
			}
		}
		for (idx=next->neighborhood; idx; idx=idx->next) {
			if (idx->endPoint == near) {
				idx->used = 1;
			}
		}

		/* remove node from Graph, check if one of the neighbors
		 * becomes degree two vertex. If it is not already in list
		 * (which is encoded in ->d), add it to list.
		 *
		 * TODO ?
		 * BUG: not degree two vertex, but 2-vertex. this is a difference
		 * But 2-vertices make no sense in the context of ops */
		removeEdge(near, v, sgp->listPool);
		if (isDegreeTwoVertex(near)) {
			if (!near->d) {
				struct VertexList* e = getVertexList(sgp->listPool);
				e->endPoint = near;
				/* printf("add %i to list of deg 2 vertices\n", near->number); */
				appendEdge(list, e);
				near->d = 1;
			}
		}

		removeEdge(next, v, sgp->listPool);
		if (isDegreeTwoVertex(next)) {
			if (!next->d) {
				struct VertexList* e = getVertexList(sgp->listPool);
				/* printf("add %i to list of deg 2 vertices\n", next->number); */
				e->endPoint = next;
				appendEdge(list, e);
				next->d = 1;
			}
		}


		/* Here, the vertex should be removed from the graph. But the edges in
		 * hamiltonianCycle point to the vertices. We need them for the computation
		 * of the actual cycle. so we only dump the neighborhood of v. */
		dumpVertexListRecursively(sgp->listPool, v->neighborhood);
		v->neighborhood = NULL;


		/* this part may be omitted as it seems to be the case that
		 * "searching the list of pairs for an occurrence of something that
		 * is not in the list of edges includes an abort if there are e.g.
		 * two copies of an edge in pairs, but only one copy f that edge in
		 * edges.
		 *
		/ * mistake in the paper. one has to check, if the new edge
		 * is not contained in more than two triangles. As one triangle
		 * is already deleted by removing v there can be only one other
		 * triangle left without violation of this condition in the former
		 * graph */
		if (commonNeighborCount(near, next) > 1) {
			/* printf("removing node %i creates an edge that lies on more than two triangles\n", node->endPoint->number); */
			dumpShallowGraph(sgp, list);
			dumpShallowGraph(sgp, pairs);
			dumpShallowGraph(sgp, edges);
			dumpShallowGraph(sgp, hamiltonianCycle);
			dumpShallowGraph(sgp, diagonals);
			return NULL;
		}



		/* at this point, node->endPoint should be dumped, but this is not necessary
		 * as it is not considered any more and can be handled when dumping the whole
		 * graph
		dumpVertexList(sgp->listPool, popEdge(list)); */

		if (list->m - i < 2) {
			/* printf("removing node %i leaves not enough degree 2 vertices\n", node->endPoint->number); */
			dumpShallowGraph(sgp, list);
			dumpShallowGraph(sgp, pairs);
			dumpShallowGraph(sgp, edges);
			dumpShallowGraph(sgp, hamiltonianCycle);
			dumpShallowGraph(sgp, diagonals);
			return NULL;
		}
	}

	/* add the last edge (near, next) to edges */
	appendEdge(edges, shallowCopyEdge(pair, sgp->listPool));

	/* check for occurrence of something in pairs, that is not in edges
	 * for this, sort the two lists in lexicographic order and sweep through
	 * the two arrays afterwards */
	edgeArray = malloc(edges->m * sizeof(struct VertexList*));
	for (i=0, idx=edges->edges; i<edges->m; ++i, idx=idx->next) {
		edgeArray[i] = idx;
	}

	pairArray = malloc(pairs->m * sizeof(struct VertexList*));
	for (i=0, idx=pairs->edges; i<pairs->m; ++i, idx=idx->next) {
		pairArray[i] = idx;
	}

	qsort(pairArray, pairs->m, sizeof(struct VertexList*), &compareDirectedEdges);
	qsort(edgeArray, edges->m, sizeof(struct VertexList*), &compareDirectedEdges);


	/* sweep */
	for (j=0, k=0, found=1; j<pairs->m; ++j) {

		/* increment currentEdge as long as it is lex. smaller than sweeper. */
		for (; compareDirectedEdges(&(pairArray[j]), &(edgeArray[k])) > 0; ++k);

		/* check if the two are equal */
		if (compareDirectedEdges(&(pairArray[j]), &(edgeArray[k])) == 0) {
			++k;
			continue;

		} else {
			/* this is bad, g is no mop */
			found = 0;
			break;
		}
	}

	/* preliminary garbage collection */
	dumpShallowGraph(sgp, list);
	dumpShallowGraph(sgp, pairs);
	dumpShallowGraph(sgp, edges);
	free(edgeArray);
	free(pairArray);

	/* create canonical string of this block or return NULL,
	 * if the block is not outerplanar */
	if (found) {
		struct VertexList* e;
		struct VertexList* lastEdge = NULL;

		/* delete all edges that remain in g. That is exactly one. Store one copy of it
		 * in case that this edge belongs to the hamiltonian cycle. */
		for (i=0; i<g->n; ++i) {
			if (g->vertices[i]->neighborhood) {
				dumpVertexListRecursively(sgp->listPool, lastEdge);
				lastEdge = g->vertices[i]->neighborhood;
				g->vertices[i]->neighborhood = NULL;
			}
		}

		/* add edges in hamiltonianCycle to g */
		while ((e = popEdge(hamiltonianCycle))) {
			e->next = e->startPoint->neighborhood;
			e->startPoint->neighborhood = e;

			e = inverseEdge(e, sgp->listPool);
			e->next = e->startPoint->neighborhood;
			e->startPoint->neighborhood = e;
		}

		/* hamiltonianCycle is now empty and g is a undirected cycle.
		 * Now obtain a ShallowGraph where edges are ordered correctly. */
		for (e=g->vertices[0]->neighborhood; e; e=e->endPoint->neighborhood) {

			/* remove e from the adjacency list of its startpoint */
			e->startPoint->neighborhood = e->next;
			e->next = NULL;

			/* remove inverse edge of e from adjacency list of e's endpoint
			 * remember: a vertex has exactly two neighbors except in the
			 * last iteration of this loop */
			if (e->endPoint->neighborhood->endPoint == e->startPoint) {
				struct VertexList* tmp = e->endPoint->neighborhood;
				e->endPoint->neighborhood = e->endPoint->neighborhood->next;
				dumpVertexList(sgp->listPool, tmp);
			} else {
				dumpVertexList(sgp->listPool, e->endPoint->neighborhood->next);
				e->endPoint->neighborhood->next = NULL;
			}

			/* add e to hamiltonianCycle */
			appendEdge(hamiltonianCycle, e);
		}

		/* in case the reducion phase of this algorithm reduced the last triangle
		 * onto an edge on the hamiltonian cycle of g, this edge is not yet contained
		 * in hamiltonianCycle. We have stored it in lastEdge. Add it. */
		if (hamiltonianCycle->edges->startPoint != hamiltonianCycle->lastEdge->endPoint) {
			if (hamiltonianCycle->lastEdge->endPoint == lastEdge->startPoint) {
				appendEdge(hamiltonianCycle, lastEdge);
			} else {
				appendEdge(hamiltonianCycle, inverseEdge(lastEdge, sgp->listPool));
				dumpVertexList(sgp->listPool, lastEdge);
			}
		} else {
			dumpVertexList(sgp->listPool, lastEdge);
		}

		/* special case:
		 * TODO needs theoretical background
		 * if diagonals contains exactly one edge, it may be the case that this edge is
		 * contained in the hamiltonian cycle and has to be removed.
		 * If so, this edge has been lastEdge above and is stored in
		 * hamiltonianCycle->lastEdge, we have to check for inverse edges, too. */
		if (diagonals->m == 1) {
			if ((diagonals->edges->startPoint == hamiltonianCycle->lastEdge->startPoint)
					&& (diagonals->edges->endPoint == hamiltonianCycle->lastEdge->endPoint)) {
				dumpVertexList(sgp->listPool, popEdge(diagonals));
			}
			if ((diagonals->edges->startPoint == hamiltonianCycle->lastEdge->endPoint)
					&& (diagonals->edges->endPoint == hamiltonianCycle->lastEdge->startPoint)) {
				dumpVertexList(sgp->listPool, popEdge(diagonals));
			}
		}

		/* now hamiltonianCycle is a cycle and
		 * diagonals contains the diagonals of the block. hurra! */
		hamiltonianCycle->next = diagonals;
		return hamiltonianCycle;


	} else {
		/* g is not outerplanar. return NULL. */
		dumpShallowGraph(sgp, hamiltonianCycle);
		dumpShallowGraph(sgp, diagonals);
		return NULL;
	}

	return NULL;
}



struct ShallowGraph* canonicalStringOfOuterplanarGraph(struct ShallowGraph* original, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct Graph* g = shallowGraphToGraph(original, gp);
	struct ShallowGraph* hamiltonianCycle = __getCycleAndDiagonals(g, sgp);
	if (hamiltonianCycle != NULL) {
		struct ShallowGraph* diagonals = hamiltonianCycle->next;	
		struct ShallowGraph* cString;
		hamiltonianCycle->next = NULL;
		cString = canonicalStringOfOuterplanarBlock(hamiltonianCycle, diagonals, sgp);
		dumpGraph(gp, g);
		return cString;
	} else {
		dumpGraph(gp, g);
		return NULL;
	}
}
