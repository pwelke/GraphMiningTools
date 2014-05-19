#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>

#include "../graph.h"
#include "../searchTree.h"
#include "../loading.h"
#include "../listSpanningTrees.h"
#include "../upperBoundsForSpanningTrees.h"
#include "../subtreeIsomorphism.h"
#include "../graphPrinting.h"
#include "../treeCenter.h"
#include "../connectedComponents.h"
#include "../cs_Tree.h"
#include "../wilsonsAlgorithm.h"

char DEBUG_INFO = 1;

/**
 * Print --help message
 */
void printHelp() {
	printf("This is The TreePatternKernel\n");
	printf("implemented by Pascal Welke 2013\n\n\n");
	printf("usage: tpk F [parameterList]\n\n");
	printf("    without parameters: display this help screen\n\n");
	printf("    F: (required) use F as graph database\n\n");
	printf("    -bound d: choose maximum number of spanning trees\n"
		   "        of a graph for which you are willing to compute\n"
		   "        the kernel (default 1000)\n\n");
	printf("    -label L: change labels\n\n");
	printf("        n \"nothing\" (default) do not change anything\n"
		   "        a \"active\" active - 1 | moderately active, inactive - -1\n"
		   "        i \"CA vs. CI\" active - 1 | inactive - -1 moderately active removed\n"
		   "        m \"moderately active\" active, moderately active  - 1 | inactive - -1\n\n");
	printf("    -output O: write output to stdout (default p)\n"
		   "        e returns the estimated number of spanning trees\n"
		   "        s returns true number of spanning trees or -1 if\n"
		   "            there are more than bound\n"
		   "        c return if graph is connected\n"
		   "        p print the spanning tree patterns of all graphs with\n"
		   "            less than filter spanning trees\n");
	printf("    -limit N: process the first N graphs in F (default: process all)\n");
	printf("    -min M process graphs starting from Mth instance (default 0)\n\n");
	printf("    -h | --help: display this help\n\n");
}


/**
 * Change label of graph according to -label input parameter.
 */
void labelProcessing(struct Graph* g, char p) {

	switch (p) {
	/* change nothing */
	case 'n':
		break;
	case 0:
		break;
	/* active */
	case 'a':
		if (g->activity == 2) {
			g->activity = 1;
		} else {
			g->activity = -1;
		}
		break;
	/* moderately active */
	case 'm':
		if (g->activity == 0) {
			g->activity = -1;
		}
		if (g->activity == 2) {
			g->activity = 1;
		}
		break;
	}
}


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
int main(int argc, char** argv) {
	if ((argc < 2) || (strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)) {
		printHelp();
		return EXIT_FAILURE;
	} else {

		/* time measurements */
		clock_t tic = clock();
		clock_t toc;

		/* create object pools */
		struct ListPool *lp = createListPool(1);
		struct VertexPool *vp = createVertexPool(1);
		struct ShallowGraphPool *sgp = createShallowGraphPool(1, lp);
		struct GraphPool *gp = createGraphPool(1, vp, lp);

		/* pointer to the current graph which is returned by the input iterator */
		struct Graph* g = NULL;

		/* user input handling variables */
		char outputOption = 0;
		char labelOption = 0;
		int param;
		long int depth = 1000;

		/* i counts the number of graphs read */
		int i = 0;

		/* graph delimiter */
		int maxGraphs = -1;
		int minGraph = 0;


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
			if (strcmp(argv[param], "-label") == 0) {
				labelOption = argv[param+1][0];
			}
			if (strcmp(argv[param], "-bound") == 0) {
				sscanf(argv[param+1], "%li", &depth);
				if (depth < 1) {
					depth = LONG_MAX / 2;
				}
			}
			if (strcmp(argv[param], "-min") == 0) {
				sscanf(argv[param+1], "%i", &minGraph);
			}
		}

		if (outputOption == 0) {
			outputOption = 'p';
		}

		/* try to load a file */
		createFileIterator(argv[1], gp);

		/* iterate over all graphs in the database */
		while (((i < maxGraphs) || (maxGraphs == -1)) && (g = iterateFile(&aids99VertexLabel, &aids99EdgeLabel))) {
		
			/* if there was an error reading some graph the returned n will be -1 */
			if (g->n > 0) {
				if (i >= minGraph) {
					long int spanningTreeEstimate;

					/* filter out moderately active molecules, if 'i' otherwise set labels */
					if (labelOption == 'i') {
						if (g->activity == 1) {
							dumpGraph(gp, g);
							continue;
						} else {
							labelProcessing(g, 'a');
						}
					}else {
						labelProcessing(g, labelOption);
					}

					switch (outputOption) {
						case 's':
						spanningTreeEstimate = countSpanningTrees(g, depth, sgp, gp);
						fprintf(stdout, "%i %li\n", g->number, spanningTreeEstimate);
						break;

						case 'c':
						spanningTreeEstimate = isConnected(g);
						fprintf(stdout, "%i %li\n", g->number, spanningTreeEstimate);
						break;

						case 'e':
						spanningTreeEstimate = getGoodEstimate(g, sgp, gp);
						fprintf(stdout, "%i %li\n", g->number, spanningTreeEstimate);
						break;

						case 'p':
						if (isConnected(g)) {
							/* getGoodEstimate returns an upper bound on the number of spanning
							trees in g, or -1 if there was an overflow of long ints while computing */
							struct Vertex* searchTree = getVertex(gp->vertexPool);
							long upperBound = getGoodEstimate(g, sgp, gp);
							if ((upperBound < depth) && (upperBound != -1)) {
								struct ShallowGraph* trees = listSpanningTrees(g, sgp, gp);
								struct ShallowGraph* idx;
								//int st = 0;
								for (idx=trees; idx; idx=idx->next) {	
									struct Graph* tree = shallowGraphToGraph(idx, gp);

									/* assumes that tree is a tree */
									struct ShallowGraph* cString = canonicalStringOfTree(tree, sgp);
									addToSearchTree(searchTree, cString, gp, sgp);
									//++st;

									/* garbage collection */
									dumpGraph(gp, tree);
								}
								dumpShallowGraphCycle(sgp, trees);
							} else {
								/* if there are more than depth spanning trees, we sample depth of them uniformly at random */
								int i;
								for (i=0; i<depth; ++i) {
									struct ShallowGraph* treeEdges = randomSpanningTreeAsShallowGraph(g, sgp);
									struct Graph* tree = shallowGraphToGraph(treeEdges, gp);

									/* assumes that tree is a tree */
									struct ShallowGraph* cString = canonicalStringOfTree(tree, sgp);
									addToSearchTree(searchTree, cString, gp, sgp);
									//++st;

									/* garbage collection */
									dumpGraph(gp, tree);
								}
							}
							/* output tree patterns represented as canonical strings */
							printf("# %i %i\n", g->number, searchTree->d);
							printStringsInSearchTree(searchTree, stdout, sgp);
							fflush(stdout);

							dumpSearchTree(gp, searchTree);
						}
						break;
					}
				}
				
				
				if (DEBUG_INFO) {
					if (i % 500 == 0) { fprintf(stderr, "."); }
				}

				/***** do not alter ****/

				++i;
				/* garbage collection */
				dumpGraph(gp, g);

			} else {
				/* TODO should be handled by dumpgraph */
				free(g);
			}
		}

		/* global garbage collection */
		destroyFileIterator();
		freeGraphPool(gp);
		freeShallowGraphPool(sgp);
		freeListPool(lp);
		freeVertexPool(vp);

		toc = clock();
		if (outputOption == 't') {
			printf("It took %li milliseconds to process the %i graphs\n", (toc - tic) / 1000, i);
		}

		return EXIT_SUCCESS;
	}
}
