#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "graph.h"
#include "searchTree.h"
#include "listSpanningTrees.h"
#include "upperBoundsForSpanningTrees.h"
#include "graphPrinting.h"
#include "levelwiseMain.h" 
#include "levelwiseMining.h"



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
 Main method of the TreePatternKernel levelwise pattern generation algorithm.
 It will use a database of spanning trees generated by the preprocessing 
 algorithm accompanying it.
 */
int main(int argc, char** argv) {
	if ((argc < 2) || (strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)) {
		printHelp();
		return EXIT_FAILURE;
	} else {

		/* create object pools */
		struct ListPool *lp = createListPool(1);
		struct VertexPool *vp = createVertexPool(1);
		struct ShallowGraphPool *sgp = createShallowGraphPool(1, lp);
		struct GraphPool *gp = createGraphPool(1, vp, lp);

		/* user input handling variables */
		char outputOption = 0;
		int param;

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
		char debugInfo = 1;
		int minGraph = 0;
		int maxGraph = 45000;
		int threshold = (maxGraph - minGraph) / 10;
		int maxPatternSize = 4;
		int minEdgeID = 100;
		char* featureFileName = "results/features.txt";
		char* countFileName = "results/counts.txt";
		char* inputFileName = "results/2013-08-26_spanningTreePatterns.txt";
		char* patternFileName = "results/patterns.txt";

		/* internal init */
		FILE* featureFile = fopen(featureFileName, "w");
		FILE* countFile = fopen(countFileName, "w");
		FILE* patternFile = fopen(patternFileName, "w");

		struct Vertex* frequentPatterns;
		struct Vertex* frequentVertices = getVertex(vp);
		struct Vertex* frequentEdges = getVertex(vp);
		struct ShallowGraph* extensionEdges;
		int patternSize;

		/* find frequent single vertices and frequent edges */
		/* set lowest id of any edge pattern to a number large enough to don't have collisions */
		frequentEdges->lowPoint = minEdgeID;
		getVertexAndEdgeHistogramsP(inputFileName, minGraph, maxGraph, frequentVertices, frequentEdges, countFile, gp, sgp);
		filterSearchTreeP(frequentVertices, threshold, frequentVertices, featureFile, gp);
		filterSearchTreeP(frequentEdges, threshold, frequentEdges, featureFile, gp);


		/* print first two levels to patternfile */
		fprintf(patternFile, "patterns size 0\n");
		printStringsInSearchTree(frequentVertices, patternFile, sgp); 
		fprintf(patternFile, "patterns size 1\n");
		printStringsInSearchTree(frequentEdges, patternFile, sgp); 
		if (debugInfo) { fprintf(stderr, "Computation of level 0 and 1 done\n"); }

		/* convert frequentEdges to ShallowGraph */
		extensionEdges = edgeSearchTree2ShallowGraph(frequentEdges, gp, sgp);	

		for (frequentPatterns = frequentEdges, patternSize = 2; (frequentPatterns->d > 0) && (patternSize < maxPatternSize); ++patternSize) {
			int i;
			struct ShallowGraph* prefix = getShallowGraph(sgp);
			struct Vertex* candidateSet;
			struct Vertex** pointers;
			struct Graph** refinements;
			
			candidateSet = generateCandidateSet(frequentPatterns, extensionEdges, gp, sgp);
			setLowPoints(candidateSet);
			pointers = malloc(candidateSet->d * sizeof(struct Vertex*));
			refinements = malloc(candidateSet->d * sizeof(struct Graph*));

			makeGraphsAndPointers(candidateSet, candidateSet, refinements, pointers, 0, prefix, gp, sgp); 
			scanDB(inputFileName, candidateSet, refinements, pointers, candidateSet->d, minGraph, maxGraph, countFile, gp, sgp);

			/* threshold + 1 as candidateSet contains each candidate once, already */
			filterSearchTreeP(candidateSet, threshold + 1, candidateSet, featureFile, gp);

			fprintf(patternFile, "patterns size %i\n", patternSize + 1);
			printStringsInSearchTree(candidateSet, patternFile, sgp); 

			if (debugInfo) { fprintf(stderr, "Computation of level %i done\n", patternSize + 1); }

			/* garbage collection */
			dumpSearchTree(gp, frequentPatterns);
			dumpShallowGraph(sgp, prefix);
			free(pointers);
			for (i=0; i<candidateSet->d; ++i) {
				dumpGraph(gp, refinements[i]);
			}
			free(refinements);
			frequentPatterns = candidateSet;

			/* flush the output */
			fflush(featureFile);
			fflush(countFile);
			fflush(patternFile);
		}

		/* garbage collection */
		freeFrequentEdgeShallowGraph(gp, sgp, extensionEdges);
		dumpSearchTree(gp, frequentVertices);
		dumpSearchTree(gp, frequentPatterns);
		fclose(featureFile);
		fclose(countFile);
		fclose(patternFile);

		freeGraphPool(gp);
		freeShallowGraphPool(sgp);
		freeListPool(lp);
		freeVertexPool(vp);

		return EXIT_SUCCESS;
	}
}
