#include <string.h>

#include "cs_Compare.h"


/**
 * Compare two !directed! edges lexicographically (compatible to stdlibs qsort)
 */
int compareDirectedEdges(const void* p1, const void* p2) {
	struct VertexList* e1 = *(struct VertexList**)p1;
	struct VertexList* e2 = *(struct VertexList**)p2;

	if (e1->startPoint->number < e2->startPoint->number) {
		return -1;
	}
	if (e1->startPoint->number > e2->startPoint->number) {
		return 1;
	}
	if (e1->endPoint->number < e2->endPoint->number) {
		return -1;
	}
	if (e1->endPoint->number > e2->endPoint->number) {
		return 1;
	}
	return 0;

}


/**
 * Wrapper method for use in qsort
 */
int lexCompCS(const void* e1, const void* e2) {

	struct ShallowGraph* g = *((struct ShallowGraph**)e1);
	struct ShallowGraph* h = *((struct ShallowGraph**)e2);

	return compareCanonicalStrings(g, h);
}


/**
Computes the lexicographic order of two rooted paths given by their roots \verb+ r1+ and \verb+ r2+ recursively. 
This function returns
\[ -1 if P1 < P2 \]
\[ 0 if P1 = P2 \]
\[ 1 if P1 > P2 \]
and uses the comparison of label strings as total ordering.
The two paths are assumed to have edge labels and vertex labels are not taken into account.
 */
int compareVertexLists(const struct VertexList* e1, const struct VertexList* e2) {

	/* if this value is larger than 0 the first label is lex. larger than the second etc. */
	int returnValue = strcmp(e1->label, e2->label);

	/* if the two paths are identical so far wrt. labels check the next vertex on each path */
	if (returnValue == 0) {

		if ((e1->next) && (e2->next)) {
			/* if both lists have a next, proceed recursively */
			return compareVertexLists(e1->next, e2->next);
		} else {
			/* if the first list is shorter, its lex. smaller */
			if ((e1->next == NULL) && (e2->next)){
				return -1;
			}
			/* if the second is shorter, this one is lex.smaller */
			if ((e2->next == NULL) && (e1->next)){
				return 1;
			}
		}

		/* if none of the cases above was chosen, both paths end here and we have to return 0 as both strings
		represented by the lists are identical */
		return 0;

	} else {
		/* if the paths differ in the current vertices, we return the return value of the comparison function */
		return returnValue;
	} 
}


/**
Compare two strings represented by ShallowGraphs to each other. 
Return behaviour should match that of string.h's strcmp() function */
int compareCanonicalStrings(const struct ShallowGraph *g1, const struct ShallowGraph *g2) {
	return compareVertexLists(g1->edges, g2->edges);
}