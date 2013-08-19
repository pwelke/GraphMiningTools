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
#include "levelwiseMain.h" 
#include "levelwiseMining.h"
/* #include "opk.h" */

char DEBUG_INFO = 1;


void iterateGraphDB(char* fileName, int minGraph, int maxGraph, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct Graph* g = NULL;
	int i = 0;
	
	/* try to load a file */
	createFileIterator(fileName, gp);

	/* iterate over all graphs in the database */
	while (((i < maxGraph) || (maxGraph == -1)) && (g = iterateFile(&aids99VertexLabel, &aids99EdgeLabel))) {
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n > 0) {
			if (i > minGraph) {
				// TODO what needs to be done
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
		
}


/**
 * Print --help message
 */
void printHelp() {
	printf("This is a Levelwise Algorithm for TreePatterns\n");
	printf("implemented by Pascal Welke 2013\n\n\n");
	printf("usage: lwm F [parameterList]\n\n");
	printf("    without parameters: display this help screen\n\n");
	printf("    F: (required) use F as graph database\n\n");
	printf("    -output O: write output to stdout\n");

	printf("    -limit N: process the first N graphs in F\n\n");
	printf("    -h | --help: display this help\n\n");
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

		/* pointer to an array managed by CyclicPatternKernel to store output results */
		int imrSize = 0;

		/* user input handling variables */
		char outputOption = 0;
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
			}
			if (strcmp(argv[param], "-output") == 0) {
				outputOption = argv[param+1][0];
			}
		}

		if (outputOption == 0) {
			outputOption = 'a';
		}

		// init params
		int threshold = 2;
		int maxPatternSize = 10;
		char* featureFileName = "results/features.txt";
		char* countFileName = "results/counts.txt";
		char* inputFileName = "results/2013-07-23_spanningTreePatterns.txt";
		char* patternFileName = "results/patterns.txt";
		int minGraph = 0;
		int maxGraph = 2;

		// internal init
		struct Vertex* patterns = getVertex(vp);
		struct Vertex* frequentEdgeSearchTree = getVertex(vp);
		struct Vertex* currentLevel;
		int patternSize = 0;

		FILE* featureFile = fopen(featureFileName, "w");
		FILE* countFile = fopen(countFileName, "w");
		FILE* patternFile = fopen(patternFileName, "w");

		// debug
		fprintf(stderr, "prep done\n");

		/* find frequent single vertices and frequent edges */
		getVertexAndEdgeHistograms(inputFileName, minGraph, maxGraph, patterns, frequentEdgeSearchTree, gp, sgp);
		frequentPatterns(patterns, threshold, patterns, gp);
		frequentPatterns(frequentEdgeSearchTree, threshold, frequentEdgeSearchTree, gp);

		// debug
		fprintf(stderr, "vertices:\n");
		printStringsInSearchTree(patterns, stdout, sgp); 
		fprintf(stderr, "edges:\n");
		printStringsInSearchTree(frequentEdgeSearchTree, stdout, sgp); 


		// for (currentLevel = patterns; (currentLevel->visited > 0) && (patternSize < maxPatternSize); ++patternSize) {
		// 	dumpSearchTree(gp, patterns);
		// 	patterns = getVertex(gp->vertexPool);
			

		// 	/* search tree for two levels below is not needed, as patterns are written to file anyhow */
		// 	dumpSearchTree(gp, currentLevel);
		// }

		// debug garbage collection
		dumpSearchTree(gp, frequentEdgeSearchTree);
		dumpSearchTree(gp, patterns);

		/* garbage collection */
		fclose(featureFile);
		fclose(countFile);
		fclose(patternFile);

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
