///*
// * localEasySubtreeIsomorphism.c
// *
// *  Created on: Sep 23, 2016
// *      Author: pascal
// */
//
//
//struct Graph* getBlockTreeT(struct Graph* g, struct ShallowGraphPool* sgp) {
//
//	char** isRoot = malloc(g->n * sizeof(char))
//			int** smallestComponent = malloc(g->n * sizeof(int));
//	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
//
//	for (int v=0; v<g->n; ++v) {
//		isRoot[v] = 0;
//		smallestComponent[v] = g->n + 1;
//	}
//
//	// mark first vertex that occurs as root
//	int currentComp = 0;
//	for (struct ShallowGraph* bic=biconnectedComponents; bic!=NULL; bic=bic->next, ++currentComp) {
//		for (struct VertexList* e=bic->edges; e!=NULL; e=e->next) {
//			struct Vertex* v = e->startPoint;
//			struct Vertex* w = e->endPoint;
//
//			if (smallestComponent[v->number] < currentComp) {
//				isRoot[v->number] = 1;
//			} else {
//				smallestComponent[v->number] = currentComp;
//			}
//
//			if (smallestComponent[w->number] < currentComp) {
//				isRoot[w->number] = 1;
//			} else {
//				smallestComponent[w->number] = currentComp;
//			}
//		}
//	}
//
//	for (int v=0; v<g->n; ++v) {
//		fprintf(stdout, "%i ", g->vertices[v]->lowPoint);
//	}
//	fprintf(stdout, "\n");
//	for (int v=0; v<g->n; ++v) {
//		fprintf(stdout, "%i ", isRoot[v]);
//	}
//	fprintf(stdout, "\n");
//	for (int v=0; v<g->n; ++v) {
//		fprintf(stdout, "%i ", smallestComponent[v]);
//	}
//	fprintf(stdout, "\n");
//	fprintf(stdout, "\n");
//
//}
//
//
///**
//Return the maximum number of spanning trees in all biconnected components containing v over all
//vertices v in g.
//If the maximum is larger than INT_MAX, or the counting  did not succeed at at least one vertex,
//return -1
//*/
//int getMaxLocalEasiness(struct Graph* g, long int maxBound, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
//	int v;
//	long int max = -1;
//	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
//	long int* easiness = computeLocalEasinessExactly(biconnectedComponents, g->n, maxBound, gp, sgp);
//	for (v=0; v<g->n; ++v) {
//		// problem occurred. counting did not work
//		if (easiness[v] == 0) {
//			return -1;
//		}
//		if (easiness[v] > max) {
//			max = easiness[v];
//		}
//	}
//	return (max > (long int)INT_MAX) ? -1 : (int)max;
//}
//
//
//long int* computeLocalEasinessExactly(struct ShallowGraph* biconnectedComponents, int n, long int maxBound, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
//
//	/* store for each vertex if the current bic.comp was already counted */
//	int* occurrences = malloc(n * sizeof(int));
//	/* store the cycle degrees of each vertex in g */
//	long int* easiness = malloc(n * sizeof(long int));
//
//	int v;
//	struct ShallowGraph* comp;
//	int compNumber = 0;
//
//	for (v=0; v<n; ++v) {
//		occurrences[v] = -1;
//		easiness[v] = 1;
//	}
//
//	// fprintf(stderr, "process some graph\n");
//
//	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
//		if (comp->m > 1) {
//			struct VertexList* e;
//			struct Graph* component = shallowGraphToGraph(comp, gp);
//			// counts spanning trees in component, but breaks and returns -1 if there are more than maxBound
//			long int nSpanningTrees = countSpanningTrees(component, maxBound, sgp, gp);
//			// fprintf(stderr, "component %i: %i\n", compNumber, nSpanningTrees);
//			dumpGraph(gp, component);
//			for (e=comp->edges; e!=NULL; e=e->next) {
//				if (occurrences[e->startPoint->number] < compNumber) {
//					occurrences[e->startPoint->number] = compNumber;
//					if (nSpanningTrees != -1) {
//						easiness[e->startPoint->number] *= nSpanningTrees;
//					} else {
//						easiness[e->startPoint->number] = 0;
//					}
//				}
//				if (occurrences[e->endPoint->number] < compNumber) {
//					occurrences[e->endPoint->number] = compNumber;
//					if (nSpanningTrees != -1) {
//						easiness[e->endPoint->number] *= nSpanningTrees;
//					} else {
//						easiness[e->endPoint->number] = 0;
//					}
//				}
//			}
//			++compNumber;
//		}
//	}
//
//	// fprintf(stderr, "result: \n");
//	// for (v=0; v<n; ++v) {
//	// 	fprintf(stderr, "%i ", easiness[v]);
//	// }
//	// fprintf(stderr, "\n");
//	free(occurrences);
//	return easiness;
//}
