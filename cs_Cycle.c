#include <string.h>

#include "graph.h"
#include "cs_Parsing.h"
#include "cs_Compare.h"
#include "cs_Cycle.h"


/**********************************************************************************
 *************************** canonical strings for cycles *************************
 **********************************************************************************/


/**
 * If you want to compare two cycles by yourself, use compareShallowGraphCycles() instead.
 *
 * Compare two cycles of equal length. Caution, the input parameters point to the edges
 * BEFORE the edges starting the cycle.
 * This is due to the usage of this method in permutateCycle().
 */
struct VertexList* compareCycles(struct VertexList* c1, struct VertexList* c2) {
	struct VertexList* idx1 = c1;
	struct VertexList* idx2 = c2;

	/* check for differences in the following edges */
	for (idx1=idx1->next, idx2=idx2->next; idx1!=c1; idx1=idx1->next, idx2=idx2->next) {

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

	/* if no difference is found, return c1 */
	return c1;
}


/**
 * Compares two ShallowGraphs representing cycles of the same length and returns the one
 * resulting in the lexicographically smaller canonical string
 */
struct ShallowGraph* compareShallowGraphCycles(struct ShallowGraph* c1, struct ShallowGraph* c2) {
	struct VertexList* smallest;

	c1->lastEdge->next = c1->edges;
	c2->lastEdge->next = c2->edges;

	/* see doc of compareCycles for explanation of this call */
	smallest = compareCycles(c1->lastEdge, c2->lastEdge);

	c1->lastEdge->next = NULL;
	c2->lastEdge->next = NULL;

	return (smallest == c1->lastEdge) ? c1 : c2;

}


/**
 * Finds the cyclic permutation that results in a lexicographically smallest
 * representation of the input cycle, if one considers the canonical string beginning
 * with the edge label of the first edge in the ->edge list followed by the vertex label
 * of its endpoint etc.
 */
struct ShallowGraph* permutateCycle(struct ShallowGraph* cycle) {
	struct VertexList* start = cycle->edges;
	struct VertexList* end = assertLastPointer(cycle);
	/* best points to the edge before the starting edge of the lex smallest cycle */
	struct VertexList* best = cycle->edges;
	struct VertexList* idx;

	/* make actual cycle */
	end->next = start;

	for (idx = start->next; idx != start; idx=idx->next) {
		best = compareCycles(best, idx);
	}

	/* turn cycle s.t. the shallow graph is lex. smallest */
	cycle->edges = best->next;
	cycle->lastEdge = best;
	best->next = NULL;

	return cycle;
}

/**
 * Returns the canonical string of a cycle. This method consumes the cycle.
 */
void cycleToString(struct ShallowGraph* cycle, struct ShallowGraphPool* sgp) {
	struct VertexList* idx, *tmp;
	struct ShallowGraph* tmpString = getShallowGraph(sgp);

	appendEdge(tmpString, getInitialisatorEdge(sgp->listPool));

	for (idx=cycle->edges; idx; idx=tmp) {
		struct VertexList* endPoint = getVertexList(sgp->listPool);
		endPoint->label = idx->endPoint->label;

		tmp = idx->next;

		appendEdge(tmpString, idx);
		appendEdge(tmpString, endPoint);
	}

	appendEdge(tmpString, getTerminatorEdge(sgp->listPool));

	/* move all to cycle */
	cycle->edges = tmpString->edges;
	cycle->lastEdge = tmpString->lastEdge;
	cycle->m = tmpString->m;
	/* ->next and ->prev remain */

	/* dump tmpString */
	tmpString->edges = NULL;
	dumpShallowGraph(sgp, tmpString);
}


/**
 *  returns the canonical string of the input cycle without consuming the cycle
 */
struct ShallowGraph* getCanonicalStringOfCycle(struct ShallowGraph* cycle, struct ShallowGraphPool* sgp) {
	struct VertexList* idx;
	struct ShallowGraph* tmpString = getShallowGraph(sgp);

	appendEdge(tmpString, getInitialisatorEdge(sgp->listPool));

	for (idx=cycle->edges; idx; idx=idx->next) {
		struct VertexList* endPoint = getVertexList(sgp->listPool);
		endPoint->label = idx->endPoint->label;

		appendEdge(tmpString, shallowCopyEdge(idx, sgp->listPool));
		appendEdge(tmpString, endPoint);
	}

	appendEdge(tmpString, getTerminatorEdge(sgp->listPool));

	return tmpString;
}


/**
 * Process a list of ShallowGraphs representing cycles and returns a list of canonical strings
 * for these cycles. The input is consumed.
 */
struct ShallowGraph* getCyclePatterns(struct ShallowGraph* cycles, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* idx;
	struct ShallowGraph* result = NULL;
	struct ShallowGraph* next;

	for (idx = cycles; idx; idx=next) {
		struct ShallowGraph* tmpResult1;
		struct ShallowGraph* rev;

		next = idx->next;

		tmpResult1 = result;
		idx = permutateCycle(idx);
		rev = permutateCycle(inverseCycle(idx, sgp));

		result = compareShallowGraphCycles(idx, rev);
		result->next = tmpResult1;

		/* dump the larger of the two cycles */
		(result == idx) ? dumpShallowGraph(sgp, rev) : dumpShallowGraph(sgp, idx);

		/* convert the cycle to its canonical string */
		cycleToString(result, sgp);

	}

	return result;
}
