#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "../graph.h"
#include "../loading.h"
#include "../cpk.h"
#include "cpkMain.h"


/**
 * Print --help info to screen.
 */
void printHelp() {
	printf("This is The Cyclic Pattern Kernel\n");
	printf("implemented by Pascal Welke 2011/2012\n\n\n");
	printf("usage: cpk F [parameterList]\n\n");
	printf("    without parameters: display this help screen\n\n");
	printf("    F: use F as graph database\n\n");
	printf("	-label L: change labels\n\n");
	printf("		n \"nothing\" (default) do not change anything\n"
		   "		a \"active\" active - 1 | moderately active, inactive - 0\n"
		   "		i \"CA vs. CI\" active - 1 | inactive - -1 moderately active removed\n"
		   "		m \"moderately active\" active, moderately active  - 1 | inactive - 0\n"
		   "		d print graph ids instead of labels\n\n");
	printf("    -output O: write output to stdout\n");
	printf("        a \"all\" (default) output the feature vector for each graph in the\n"
		   "            format specified by SVMlight the label of each graph has to be\n"
		   "            either 0, 1 or -1 to be compliant with the specs of SVMlight.\n");
	printf("        x output a feature vector that only contains cyclic patterns\n");
	printf("        y output a feature vector that only contains tree patterns\n");
	printf("        c returns the number of cycles found in the graph\n");
	printf("        t returns the number of trees found in each graph\n");
	printf("        v returns the number of vertices in each graph\n");
	printf("        e returns the number of edges in each graph\n\n");
	printf("    -limit N: process the first N graphs in F\n\n");
	printf("    -h | --help: display this help\n\n");
	printf("    ");
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
	/* label is graph id */
	case 'd':
		g->activity = g->number;
		break;
	}
}


/**
 * Input handling, parsing of database and call of cpk feature extraction method.
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
		struct ListPool *lp = createListPool(10000);
		struct VertexPool *vp = createVertexPool(10000);
		struct ShallowGraphPool *sgp = createShallowGraphPool(1000, lp);
		struct GraphPool *gp = createGraphPool(100, vp, lp);

		/* global search trees for mapping of strings to numbers */
		struct Vertex* globalPatternSet = getVertex(vp);

		/* pointer to the current graph which is returned by the input iterator */
		struct Graph* g = NULL;

		/* pointer to an array managed by CyclicPatternKernel to store output results */
		struct compInfo* intermediateResults = NULL;
		int imrSize = 0;

		/* user input handling variables */
		char outputOption = 0;
		char labelOption = 0;
		int param;

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
				if (!outputOption) {
					printf("Working on at most %i graphs\n", maxGraphs);
				}
			}
			if (strcmp(argv[param], "-output") == 0) {
				outputOption = argv[param+1][0];
			}
			if (strcmp(argv[param], "-label") == 0) {
				labelOption = argv[param+1][0];
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
					case 'x':
						CyclicPatternKernel_onlyCycles(g, sgp, gp, globalPatternSet, &intermediateResults, &imrSize);
						break;
					case 'y':
						CyclicPatternKernel_onlyTrees(g, sgp, gp, globalPatternSet, &intermediateResults, &imrSize);
						break;
					default:
						CyclicPatternKernel(g, sgp, gp, outputOption, globalPatternSet, &intermediateResults, &imrSize);
						break;
				}

				++i;

				/* garbage collection */
				dumpGraph(gp, g);

			} else {
				/* TODO should be handled by dumpgraph */
				free(g);
			}
		}

		/* global garbage collection */
		free(intermediateResults);

		dumpSearchTree(gp, globalPatternSet);

		destroyFileIterator();
		freeGraphPool(gp);
		freeShallowGraphPool(sgp);
		freeListPool(lp);
		freeVertexPool(vp);

		toc = clock();
		if (!outputOption) {
			printf("It took %li milliseconds to process the %i graphs\n", (toc - tic) / 1000, i);
		}

		return EXIT_SUCCESS;
	}
}



