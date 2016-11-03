#include <malloc.h>
#include <limits.h>

#include "graph.h"
#include "listSpanningTrees.h"
#include "listComponents.h"
#include "localEasiness.h"


long int* computeLocalEasinessExactly(struct ShallowGraph* biconnectedComponents, int n, long int maxBound, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	/* store for each vertex if the current bic.comp was already counted */
	int* occurrences = malloc(n * sizeof(int));
	/* store the cycle degrees of each vertex in g */
	long int* easiness = malloc(n * sizeof(long int));

	for (int v=0; v<n; ++v) {
		occurrences[v] = -1;
		easiness[v] = 1;
	}

	int compNumber = 0;
	for (struct ShallowGraph* comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		// if component is a bridge or singleton (I guess the latter cannot happen), then we do not need to process
		// spanning trees, as there is only one. This would not change the easiness of the contained vertices.
		if (comp->m <= 1) {
			continue;
		}

		// count spanning trees in component. This breaks and returns -1 if there are more than maxBound
		struct Graph* component = shallowGraphToGraph(comp, gp);
		long int nSpanningTrees = countSpanningTrees(component, maxBound, sgp, gp);
		dumpGraph(gp, component);

		for (struct VertexList* e=comp->edges; e!=NULL; e=e->next) {
			if (occurrences[e->startPoint->number] < compNumber) {
				occurrences[e->startPoint->number] = compNumber;
				if (nSpanningTrees != -1) {
					easiness[e->startPoint->number] *= nSpanningTrees;
				} else {
					easiness[e->startPoint->number] = 0;
				}
			}
			if (occurrences[e->endPoint->number] < compNumber) {
				occurrences[e->endPoint->number] = compNumber;
				if (nSpanningTrees != -1) {
					easiness[e->endPoint->number] *= nSpanningTrees;
				} else {
					easiness[e->endPoint->number] = 0;
				}
			}
		}
		++compNumber;
	}

	free(occurrences);
	return easiness;
}

/**
Return the maximum number of spanning trees in all biconnected components containing v over all 
vertices v in g.
If the maximum is larger than INT_MAX, or the counting  did not succeed at at least one vertex, 
return -1
*/
int getMaxLocalEasiness(struct Graph* g, long int maxBound, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	long int max = -1;
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	long int* easiness = computeLocalEasinessExactly(biconnectedComponents, g->n, maxBound, gp, sgp);
	for (int v=0; v<g->n; ++v) {
		// problem occurred. counting did not work
		if (easiness[v] == 0) {
			return -1;
		}
		if (easiness[v] > max) {
			max = easiness[v];
		}
	}
	free(easiness);
	return (max > (long int)INT_MAX) ? -1 : (int)max;
}


/**
Return the minimum number of spanning trees in all biconnected components containing v, over all 
vertices v in g.
If the minimum is larger than INT_MAX, or the value could not been computed for any vertex, return -1 
*/
int getMinLocalEasiness(struct Graph* g, long int maxBound, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	long int min = -1;
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	long int* easiness = computeLocalEasinessExactly(biconnectedComponents, g->n, maxBound, gp, sgp);
	for (int v=0; v<g->n; ++v) {
		// error occurred. skip it.
		if (easiness[v] == 0) {
			continue;
		}
		if ((easiness[v] < min) || (min == -1)) {
			min = easiness[v];
		}
	}
	free(easiness);
	return (min > (long int)INT_MAX) ? -1 : (int)min;
}
