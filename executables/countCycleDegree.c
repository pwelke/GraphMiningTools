#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>

#include "../listComponents.h"
#include "../loading.h"
#include "../graphPrinting.h"
#include "../connectedComponents.h"

/**
 * Print --help message
 */
void printHelp() {
	printf("This program counts the cycle degrees of vertices in a set of graphs.\n");
	printf("The cycle degree of a vertex is the number of nontrivial biconnected \n");
	printf("components, it is contained in. The cyc.deg of a graph G is the \n");
	printf("maximum over the cycle degrees of all vertices in G. \n");
	printf("implemented by Pascal Welke 2014\n\n\n");
	printf("usage: [programName] F [parameterList]\n\n");
	printf("    without parameters: display this help screen\n\n");
	printf("    F: (required) use F as graph database\n\n");
	printf("    -output O: write output to stdout\n");
	printf("        a : cycle degrees of all vertices of each graph\n");
	printf("        m : cycle degree of each graph\n");
	printf("        c : 1 if g is a connected cactus graph\n");
	printf("            0 if g is connected, but not a cactus\n");
	printf("           -1 if g is not connected\n");
	printf("        b : number of nontrivial bic. components\n\n");
	printf("    -limit N: process the first N graphs in F\n\n");
	printf("    -h | --help: display this help\n\n");
}


/**
 Main method of the TreePatternKernel levelwise pattern generation algorithm.
 It will use a database of spanning trees generated by the preprocessing 
 algorithm accompanying it.
 */
int main(int argc, char** argv) {
	if ((argc < 2) || (strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)) {
		printHelp();
		return EXIT_FAILURE;
	} else {
		int maxGraphs = -1;
		char outputOption = 'a';
		int param;
		int i = 0;
		struct Graph* g;

		/* create object pools */
		struct ListPool *lp = createListPool(1);
		struct VertexPool *vp = createVertexPool(1);
		struct ShallowGraphPool *sgp = createShallowGraphPool(1, lp);
		struct GraphPool *gp = createGraphPool(1, vp, lp);

		/* user input handling */
		for (param=2; param<argc; param+=2) {
			if ((strcmp(argv[param], "--help") == 0) || (strcmp(argv[param], "-h") == 0)) {
				printHelp();
				return EXIT_SUCCESS;
			}
			if (strcmp(argv[param], "-limit") == 0) {
				sscanf(argv[param+1], "%i", &maxGraphs);
			}
			if (strcmp(argv[param], "-output") == 0) {
				outputOption = argv[param+1][0];
			}
		}

		/* try to load a file */
		createFileIterator(argv[1], gp);

		/* iterate over all graphs in the database */
		while (((i < maxGraphs) || (maxGraphs == -1)) && (g = iterateFile(&aids99VertexLabel, &aids99EdgeLabel))) {
		
			/* if there was an error reading some graph the returned n will be -1 */
			if (g->n > 0) {

				struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
				/* store for each vertex if the current bic.comp was already counted */
				int* occurrences = malloc(g->n * sizeof(int));
				/* store the cycle degrees of each vertex in g */
				int* cycleDegrees = malloc(g->n * sizeof(int));
			
				int v;
				struct ShallowGraph* comp;
				int compNumber = 0;

				for (v=0; v<g->n; ++v) {
					occurrences[v] = -1;
					cycleDegrees[v] = 0;
				}

				for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
					if (comp->m > 1) {
						struct VertexList* e;
						for (e=comp->edges; e!=NULL; e=e->next) {
							if (occurrences[e->startPoint->number] < compNumber) {
								occurrences[e->startPoint->number] = compNumber;
								++cycleDegrees[e->startPoint->number];
							}
							if (occurrences[e->endPoint->number] < compNumber) {
								occurrences[e->endPoint->number] = compNumber;
								++cycleDegrees[e->endPoint->number];
							}
						}
						++compNumber;
					}			
				}

				/* output the results */
				if (outputOption == 'a') {
					for (v=0; v<g->n; ++v) {
						fprintf(stdout, "%i ", cycleDegrees[v]);
					}
					fprintf(stdout, "\n");
				}

				if (outputOption == 'm') {
					int maxDegree = 0;
					for (v=0; v<g->n; ++v) {
						if (maxDegree < cycleDegrees[v]) {
							maxDegree = cycleDegrees[v];
						}
					}
					fprintf(stdout, "%i\n", maxDegree);
				}
				
				if (outputOption == 'b') {
					fprintf(stdout, "%i\n", compNumber);
				}

				if (outputOption == 'c') {
					if (isConnected(g)) {
						if (g->n - 1 + compNumber == g->m) {
							fprintf(stdout, "1\n");
						} else {
							fprintf(stdout, "0\n");
						}
					} else {
						fprintf(stdout, "-1\n");
					}
				}

				/* cleanup */
				free(occurrences);
				free(cycleDegrees);
				dumpShallowGraphCycle(sgp, biconnectedComponents);

				/***** do not alter ****/
				++i;
				/* garbage collection */
				dumpGraph(gp, g);

			} else {
				/* TODO should be handled by dumpgraph */
				free(g);
			}
		}

		/* garbage collection */
		destroyFileIterator();
		freeGraphPool(gp);
		freeShallowGraphPool(sgp);
		freeListPool(lp);
		freeVertexPool(vp);

		return EXIT_SUCCESS;
	}
}
