#include <malloc.h>
#include <string.h>

#include "graph.h"
#include "bipartiteMatching.h"
#include "subtreeIsomorphism.h"
#include "iterativeSubtreeIsomorphism.h"

void initIterativeSubtreeCheck(struct Graph* g) {
	
}


/**
Iterative Labeled Subtree Isomorphism Check. 

Implements the labeled subtree isomorphism algorithm of
Ron Shamir, Dekel Tsur [1999]: Faster Subtree Isomorphism in an iterative version:

Input:
	a text    tree g
	a pattern tree h
	the cube that was computed for some subtree h-e and g, where e is an edge to a leaf of h
	(object pool data structures)

Output:
	yes, if h is subgraph isomorphic to g, no otherwise
	the cube for h and g

*/
char subtreeCheck(struct Graph* g, int* gPostorder, struct Graph* h, struct Vertex* newVertex, int*** oldS, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* iterators */
	// int u, v;

	// newVertex is a leaf, hence there is only one incident edge, which is the newly added one.
	struct VertexList* newEdge = newVertex->neighborhood;
	struct Vertex* a = newEdge->endPoint;
	struct Vertex* b = newEdge->startPoit;

	struct Vertex* r = g->vertices[0];
	int*** newS = createCube(g->n, h->n); // rewrite
	int* postorder = getPostorder(g, r->number); // refactor out

	// /* init the S(v,u) for v and u leaves */
	// int* gLeaves = findLeaves(g, 0); // refactor out
	// /* h is not rooted, thus every vertex with one neighbor is a leaf */
	// int* hLeaves = findLeaves(h, -1);
	// for (v=1; v<gLeaves[0]; ++v) {
	// 	for (u=1; u<hLeaves[0]; ++u) {
	// 		/* check compatibility of leaf labels */
	// 		if (labelCmp(g->vertices[gLeaves[v]]->label, h->vertices[hLeaves[u]]->label) == 0) {
	// 			/* check for compatibility of edges */
	// 			if (labelCmp(g->vertices[gLeaves[v]]->neighborhood->label, h->vertices[hLeaves[u]]->neighborhood->label) == 0) {
	// 				S[gLeaves[v]][hLeaves[u]] = malloc(2 * sizeof(int));
	// 				/* 'header' of array stores its length */
	// 				S[gLeaves[v]][hLeaves[u]][0] = 2;
	// 				/* the number of the unique neighbor of u in h*/
	// 				S[gLeaves[v]][hLeaves[u]][1] = h->vertices[hLeaves[u]]->neighborhood->endPoint->number;
	// 			}
	// 		}
	// 	}
	// }
	// /* garbage collection for init */
	// free(gLeaves);
	// free(hLeaves);
	// gLeaves = NULL;
	// hLeaves = NULL;

	char foundIso = 0;
	for (int vi=0; vi<g->n; ++vi) {
		struct Vertex* v = g->vertices[postorder[vi]];
		for (int ui=0; ui<h->n; ++ui) {
			struct Vertex* u = h->vertices[ui];
			int degU = degree(u); 
			addCharacteristic(newS, a, b, v);
			if (containsCharacteristic(oldS, a, a, v)) {
				addCharacteristic(newS, b, a, v);
			}
			struct Graph* B = makeBipartiteInstance(g, v->number, h, u->number, newS, gp);
			int bbCharacteristic = bipartiteMatchingFastAndDirty(B, gp);
			if (bbCharacteristic = degU) {
				addCharacteristic(newS, b, b, v);
			}
			foundIso = containsCharacteristic(newS, b, b, v);
			for (int i=1; i<degU+1; ++i) {
				if (oldS[v->number][u->number][i] == 1) {
					computeCharacteristics()
				}
			}
		}
	}


	for (v=0; v<g->n; ++v) {
		struct Vertex* current = g->vertices[postorder[v]];
		int currentDegree = degree(current);
		// if ((currentDegree != 1) || (current->number == r->number)) {
			for (u=0; u<h->n; ++u) {
				int i;
				int degU = degree(h->vertices[u]);
				if (degU <= currentDegree + 1) {
					/* if vertex labels match */
					if (labelCmp(h->vertices[u]->label, current->label) == 0) {
						struct Graph* B = makeBipartiteInstance(g, postorder[v], h, u, S, gp);
						int* matchings = malloc((degU + 1) * sizeof(int));

						matchings[0] = bipartiteMatchingFastAndDirty(B, gp);

						if (matchings[0] == degU) {
							free(postorder);
							free(matchings);
							freeCube(S, g->n, h->n);
							dumpGraph(gp, B);
							return 1;
						} else {
							matchings[0] = degU + 1;
						}

						for (i=0; i<B->number; ++i) {
							/* if the label of ith child of u is compatible to the label of the parent of v */
							if ((current->lowPoint != -1) 
								&& (labelCmp(h->vertices[B->vertices[i]->lowPoint]->label, g->vertices[current->lowPoint]->label) == 0)) {
								struct ShallowGraph* storage = removeVertexFromBipartiteInstance(B, i, sgp);
								initBipartite(B);
								matchings[i+1] = bipartiteMatchingFastAndDirty(B, gp);

								if (matchings[i+1] == degU - 1) {
									matchings[i+1] = B->vertices[i]->lowPoint;
								} else {
									matchings[i+1] = -1;
								}

								addVertexToBipartiteInstance(storage);
								dumpShallowGraph(sgp, storage);
							} else {
								matchings[i+1] = -1;
							}
						}
						S[current->number][u] = matchings;

						dumpGraph(gp, B);
					}
				}
			}		
		// }
	}

	/* garbage collection */
	free(postorder);
	freeCube(S, g->n, h->n);

	return 0;
}

void computeCharacteristics() {
	
	matchings[0] = bipartiteMatchingFastAndDirty(B, gp);

	// check if it makes sense to search for critical vertices
	if (matchings[0] >= degU - 1) {
		/* the maximum matching computed above covers all but one neighbor of u
		we need to identify those covered neighbors that can be swapped with 
		that uncovered neighbor without decreasing the cardinality of the matching
		these are exactly the non-critical vertices.

		a vertex is critical <=> 1.) AND NOT 2.)
		hence
		a vertex is non-critical <=> NOT 1.) OR 2.)
		where
		1.) matched in the matching above
		2.) reachable by augmenting path from the single unmatched vertex. 
		This means, all vertices reachable from the single uncovered neighbor of u (including that neighbor are non-critical.
		*/
		struct Vertex* uncoveredNeighbor = NULL;
		
		// find the single uncovered neighbor of u
		for (i=0; i<B->number; ++i) {
			if (!isMatched(B->vertices[i])) {
				uncoveredNeighbor = B->vertices[i];
				break;
			}			
		}

		// mark all vertices reachable from uncoveredNeighbor by an augmenting path
		markReachable(uncoveredNeighbor, B->number);

		// add non-critical vertices to output
		matchings[0] = degU + 1;
		for (i=0; i<B->number; ++i) {
			if (B->vertices[i]->visited == 1) {
				// vertex is not critical
				matchings[i+1] = B->vertices[i]->lowPoint;
			} else {
				// vertex critical
				matchings[i+1] = -1;
			}
		}

	} else {
		// makes no sense to look for critical vertices as there cannot be a matching covering all but one neighbor of u
		matchings[0] = degU + 1;
		for (i=1; i<degU + 1; ++i) {
			matchings[i] = -1;
		}
	}
	return matchings;
}
