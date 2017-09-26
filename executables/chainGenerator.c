#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "../graph.h"
#include "../graphPrinting.h"
#include "../randomGraphGenerators.h"
#include "chainGenerator.h"


/**
 * Print --help message
 */
static int printHelp() {
#include "chainGeneratorHelp.help"
	unsigned char* help = executables_chainGeneratorHelp_txt;
	int len = executables_chainGeneratorHelp_txt_len;
	if (help != NULL) {
		int i=0;
		for (i=0; i<len; ++i) {
			fputc(help[i], stdout);
		}
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Could not read help file\n");
		return EXIT_FAILURE;
	}
}


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
int main(int argc, char** argv) {

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	int i = 0;

	/* output */
	FILE* out = stdout;

	/* user set variables to specify what needs to be done */
	int numberOfGeneratedGraphs = 100;
	int lowerBoundBlocks = 1;
	int upperBoundBlocks = 50;
	int blockSize = 5;
	int nVertexLabels = -1;
	int nEdgeLabels = 1;
	int randomSeed = time(NULL);
	double diagonalProbability = 0.0;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hs:a:b:c:d:N:m:p:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
		case 's':
			if (sscanf(optarg, "%i", &randomSeed) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'a':
			if (sscanf(optarg, "%i", &lowerBoundBlocks) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'b':
			if (sscanf(optarg, "%i", &upperBoundBlocks) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'c':
			if (sscanf(optarg, "%i", &nVertexLabels) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'd':
			if (sscanf(optarg, "%i", &nEdgeLabels) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'N':
			if (sscanf(optarg, "%i", &numberOfGeneratedGraphs) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'm':
			if (sscanf(optarg, "%i", &blockSize) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'p':
			if (sscanf(optarg, "%lf", &diagonalProbability) != 1) {
				fprintf(stderr, "value must be float, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case '?':
			return EXIT_FAILURE;
			break;
		}
	}

	/* set initial random seed */
	srand(randomSeed);


	/* init object pools */
	lp = createListPool(10000);
	vp = createVertexPool(10000);
	sgp = createShallowGraphPool(1000, lp);
	gp = createGraphPool(1, vp, lp);

	/* iterate over all graphs in the database */
	for (i=0; i<numberOfGeneratedGraphs; ++i) {
		struct Graph* g;
		int nBlocks = rand() % (upperBoundBlocks - lowerBoundBlocks) + lowerBoundBlocks;
		/* calculate p, if m option was given */
		g = blockChainGenerator(nBlocks, blockSize, nVertexLabels, nEdgeLabels, diagonalProbability, gp);
		g->number = i+1;
		printGraphAidsFormat(g, out);
	}

	/* terminate output stream */
	fprintf(out, "$\n");


	/* global garbage collection */
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}
