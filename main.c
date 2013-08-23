#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>

#include "graph.h"
#include "canonicalString.h"
#include "searchTree.h"
#include "loading.h"
#include "listSpanningTrees.h"
#include "upperBoundsForSpanningTrees.h"
#include "subtreeIsomorphism.h"
#include "graphPrinting.h"
#include "treeCenter.h"
#include "connectedComponents.h"
#include "main.h" 

char DEBUG_INFO = 1;

int MINGRAPH = 0;

/**
 * Print --help message
 */
void printHelp() {
	printf("This is The TreePatternKernel\n");
	printf("implemented by Pascal Welke 2013\n\n\n");
	printf("usage: tpk F [parameterList]\n\n");
	printf("    without parameters: display this help screen\n\n");
	printf("    F: (required) use F as graph database\n\n");
	printf("    -filter d: choose maximum number of spanning trees\n"
		   "        of a graph for which you are willing to compute\n"
		   "        the kernel (default 1000)\n\n");
	printf("    -label L: change labels\n\n");
	printf("        n \"nothing\" (default) do not change anything\n"
		   "        a \"active\" active - 1 | moderately active, inactive - -1\n"
		   "        i \"CA vs. CI\" active - 1 | inactive - -1 moderately active removed\n"
		   "        m \"moderately active\" active, moderately active  - 1 | inactive - -1\n\n");
	printf("    -output O: write output to stdout\n");
	/*printf("        a \"all\" (default) output the feature vector for each graph in the\n"
		   "            format specified by SVMlight the label of each graph has to be\n"
		   "            either 0, 1 or -1 to be compliant with the specs of SVMlight.\n");*/
	printf(/*"        o returns if a graph is outerplanar\n"*/
		   "        e returns the estimated number of spanning trees\n"
		   "        s returns true number of spanning trees or -1 if\n"
		   "            there are more than depth\n"
		   /*"        t returns the time spent to compute the patterns\n"
		   "        b returns the number of vertices in the BBTree\n"
		   "        d returns the numbers of diagonals in each block\n\n"*/);

	printf("    -limit N: process the first N graphs in F\n\n");
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
			if (strcmp(argv[param], "-filter") == 0) {
				sscanf(argv[param+1], "%li", &depth);
				if (depth < 1) {
					depth = LONG_MAX / 2;
				}
			}
		}

		if (outputOption == 0) {
			outputOption = 'a';
		}

		/* try to load a file */
		createFileIterator(argv[1], gp);

		/* iterate over all graphs in the database */
		while (((i < maxGraphs) || (maxGraphs == -1)) && (g = iterateFile(&aids99VertexLabel, &aids99EdgeLabel))) {
		
			/* if there was an error reading some graph the returned n will be -1 */
			if (g->n > 0) {
				// TODO make a parameter
				if (i > MINGRAPH) {
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
						case 'e':
						spanningTreeEstimate = getGoodEstimate(g, sgp, gp);
						fprintf(stdout, "%i %li\n", g->number, spanningTreeEstimate);
						break;
						case 't':
						if (isConnected(g)) {
							if (getGoodEstimate(g, sgp, gp) < depth) {

								struct ShallowGraph* trees = listSpanningTrees(g, sgp, gp);
								struct ShallowGraph* idx;

								for (idx=trees; idx; idx=idx->next) {			
									struct Graph* h = shallowGraphToGraph(idx, gp);
									struct Graph* i = shallowGraphToGraph(idx, gp);

									char isSubgraph = subtreeCheck(h, i, gp, sgp);

									if (!isSubgraph) {
										printf("is no subgraph\n");
									}

									dumpGraph(gp, h);
									dumpGraph(gp, i);
								}
								dumpShallowGraphCycle(sgp, trees);

							}
						}
						break; 
						case 'p':
						if (isConnected(g)) {
							// fprintf(stderr, "%i\n", g->number);
							// // fprintf(stderr, "c");
							// fflush(stderr);

							/* getGoodEstimate returns an upper bound on the number of spanning
							trees in g, or -1 if there was an overflow of long ints while computing */
							long int upperBound = getGoodEstimate(g, sgp, gp);
							if ((upperBound < depth) && (upperBound != -1)) {
								// // fprintf(stderr, "e");
								// fflush(stderr);

								struct Vertex* searchTree = getVertex(gp->vertexPool);
								//long int c = countSpanningTrees(g, 1000, sgp, gp);
								// fprintf(stderr, "%li", c);
								// fflush(stderr);

								struct ShallowGraph* trees = listSpanningTrees(g, sgp, gp);
								// fprintf(stderr, "t");
								// fflush(stderr);

								struct ShallowGraph* idx;
								int st = 0;
								for (idx=trees; idx; idx=idx->next) {	
									struct Graph* tree = shallowGraphToGraph(idx, gp);

									/* assumes that tree is a tree */
									struct ShallowGraph* cString = treeCenterCanonicalString(tree, sgp);
									addToSearchTree(searchTree, cString, gp, sgp);
									++st;

									/* garbage collection */
									dumpGraph(gp, tree);
								}
								dumpShallowGraphCycle(sgp, trees);
								// fprintf(stderr, "s");
								// fflush(stderr);
								// fprintf(stderr, "%i\t%i\t%i\t%i\n", i, g->number, st, searchTree->d);
								// fflush(stderr);


								printf("# %i %i\n", g->number, searchTree->d);
								printStringsInSearchTree(searchTree, stdout, sgp);
								fflush(stdout);
								dumpSearchTree(gp, searchTree);
							}
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
