#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../graphPrinting.h"



struct Graph* erdosRenyi(int n, double p, struct GraphPool* gp) {
	int i;
	int j;
	struct Graph* g = createGraph(n, gp);
	for (i=0; i<n; ++i) {
		for (j=i+1; j<n; ++j) {
			double value = rand() / (RAND_MAX + 1.0);
			if (value < p) {
				addEdgeBetweenVertices(i, j, NULL, g, gp);
			}
		}
	}
	return g;
}


static void randomVertexLabels(struct Graph* g, int nVertexLabels) {
	int i;
	for (i=0; i<g->n; ++i) {
		g->vertices[i]->label = intLabel(rand() % nVertexLabels);
		g->vertices[i]->isStringMaster = 1;
	}
}


struct Graph* erdosRenyiWithLabels(int n, double p, int nVertexLabels, int nEdgeLabels, struct GraphPool* gp) {
	int i;
	int j;
	struct Graph* g = createGraph(n, gp);
	randomVertexLabels(g, nVertexLabels);

	for (i=0; i<n; ++i) {
		for (j=i+1; j<n; ++j) {
			double value = rand() / (RAND_MAX + 1.0);
			if (value < p) {
				// add a labeled edge and set one of the two resulting vertex lists as string master.
				addEdgeBetweenVertices(i, j, intLabel(rand() % nEdgeLabels), g, gp);
				g->vertices[i]->neighborhood->isStringMaster = 1;
			}
		}
	}
	return g;
}





/**
 * Print --help message
 */
static int printHelp() {
#include "generatorHelp.help"
	unsigned char* help = executables_generatorHelp_txt;
	int len = executables_generatorHelp_txt_len;
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
	int lowerBoundVertices = 1;
	int upperBoundVertices = 50;
	int nVertexLabels = 5;
	int nEdgeLabels = 1;
	double edgeProbability = 0.5;
	int randomSeed = 100;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hp:s:a:b:N:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
		case 'p':
			if (sscanf(optarg, "%lf", &edgeProbability) != 1) {
				fprintf(stderr, "value must be double, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 's':
			if (sscanf(optarg, "%i", &randomSeed) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'a':
			if (sscanf(optarg, "%i", &lowerBoundVertices) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'b':
			if (sscanf(optarg, "%i", &upperBoundVertices) != 1) {
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

	/* initialize the stream to read graphs from 
	   check if there is a filename present in the command line arguments 
	   if so, open the file, if not, read from stdin */
	if (optind < argc) {
		char* filename = argv[optind];
		/* if the present filename is not '-' then init a file iterator for that file name */
		if (strcmp(filename, "-") != 0) {
			createFileIterator(filename, gp);
		} else {
			createStdinIterator(gp);
		}
	} else {
		createStdinIterator(gp);
	}

	/* iterate over all graphs in the database */
	for (i=0; i<numberOfGeneratedGraphs; ++i) {
		int n = rand() % (upperBoundVertices - lowerBoundVertices) + lowerBoundVertices;
		struct Graph* g = erdosRenyiWithLabels(n, edgeProbability, nVertexLabels, nEdgeLabels, gp);
		g->number = i;
		printGraphAidsFormat(g, out);
	}

	/* terminate output stream */
	fprintf(out, "$\n");


	/* global garbage collection */
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}	
