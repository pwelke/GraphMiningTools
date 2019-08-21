#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "../graph.h"
#include "../loading.h"
#include "../graphPrinting.h"
#include "../randomGraphGenerators.h"


/**
 * Print --help message
 */
static int printHelp() {
#include "../o/help/generatorHelp.help"
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


typedef enum {
	overlap, erdosrenyi, barabasialbert, chains, iterativeoverlap, barabasialbertalpha, clusteredOverlap, iterativeClusteredOverlap
} Generator;

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
	double pParameter = 0.5;
	double mParameter = -1;
	double qParameter = 0.1;
	int randomSeed = time(NULL);
	Generator generator = erdosrenyi;
	char outputDot = 0;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "ohs:a:b:c:d:N:x:m:p:q:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		// common / global variables
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
		case 'x':
			if (strcmp(optarg, "overlap") == 0) {
				generator = overlap;
				break;
			}
			if (strcmp(optarg, "iterativeOverlap") == 0) {
				generator = iterativeoverlap;
				break;
			}
			if (strcmp(optarg, "clusteredOverlap") == 0) {
				generator = clusteredOverlap;
				break;
			}
			if (strcmp(optarg, "iterativeClusteredOverlap") == 0) {
				generator = iterativeClusteredOverlap;
				break;
			}
			if (strcmp(optarg, "erdosRenyi") == 0) {
				generator = erdosrenyi;
				break;
			}
			if (strcmp(optarg, "barabasiAlbert") == 0) {
				generator = barabasialbert;
				break;
			}
			if (strcmp(optarg, "barabasiAlbertAlpha") == 0) {
				generator = barabasialbertalpha;
				break;
			}
			if (strcmp(optarg, "chains") == 0) {
				generator = chains;
				break;
			}
			fprintf(stderr, "Unknown random generator: %s\n", optarg);
			return EXIT_FAILURE;
		case 'o':
			outputDot = 1;
			break;
		// multi purpose variables depending on generators
		case 'p':
			if (sscanf(optarg, "%lf", &pParameter) != 1) {
				fprintf(stderr, "p: value must be double, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'm':
			if (sscanf(optarg, "%lf", &mParameter) != 1) {
				fprintf(stderr, "m: value must be double, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'q':
			if (sscanf(optarg, "%lf", &qParameter) != 1) {
				fprintf(stderr, "q: value must be double, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;

		// unknown flag handling
		case '?':
			fprintf(stderr, "unknown flag: -%s\n", optarg);
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

	struct Graph* g = NULL;

	/* iterate over all graphs in the database */
	for (i=0; i<numberOfGeneratedGraphs; ++i) {
		int n = rand() % (upperBoundVertices - lowerBoundVertices) + lowerBoundVertices;

		/* calculate p, if m option was given */
		switch (generator) {
		struct Graph* core;
		case erdosrenyi:
			if ((mParameter > 0) && (n > 1)) {
				pParameter = ( 2.0 * mParameter ) / (n - 1.0);
			}
			g = erdosRenyiWithLabels(n, pParameter, nVertexLabels, nEdgeLabels, gp);
			break;
		case overlap:
			g = randomOverlapGraphWithLabels(n, pParameter, nVertexLabels, gp);
			break;
		case iterativeoverlap:
			if (i == 0) {
				g = randomOverlapGraphWithLabels(n, pParameter, nVertexLabels, gp);
			} else {
				moveOverlapGraph(g, mParameter, pParameter, gp);
			}
			break;
		case clusteredOverlap:
			g = randomClusteredOverlapGraphWithLabels(n, pParameter, nVertexLabels, qParameter, gp);
			break;
		case iterativeClusteredOverlap:
			if (i == 0) {
				g = randomClusteredOverlapGraphWithLabels(n, pParameter, nVertexLabels, qParameter, gp);
			} else {
				moveOverlapGraph(g, mParameter, pParameter, gp);
			}
			break;
		case barabasialbert:
			core = erdosRenyiWithLabels((int)pParameter, 0, 1, 1, gp);
//			core = blockChainGenerator(1, (int)edgeProbability, 1, 1, 0, gp);
			makeMinDegree1(core, gp);
			g = barabasiAlbert(n, (int)pParameter, core, gp);
			dumpGraph(gp, core);
			break;
		case barabasialbertalpha:
			core = erdosRenyiWithLabels((int)pParameter, 0, 1, 1, gp);
//			core = blockChainGenerator(1, (int)edgeProbability, 1, 1, 0, gp);
			makeMinDegree1(core, gp);
			g = barabasiAlpha(n, (int)pParameter, 0.95, core, gp);
			dumpGraph(gp, core);
			break;
		case chains:
//			g = blockChainGenerator(numberOfVertices, , nVertexLabels, nEdgeLabels, edgeProbability, gp);
			break;
		}
		g->number = i+1;

		if (outputDot) {
			if (generator == overlap || generator == iterativeoverlap || generator == clusteredOverlap || generator == iterativeClusteredOverlap) {
				printOverlapGraphDotFormat(g, out);
			} else {
				printGraphDotFormat(g, out);
			}
		} else {
			printGraphAidsFormat(g, out);
		}

		if (!(generator == iterativeoverlap || generator == iterativeClusteredOverlap)) {
			dumpGraph(gp, g);
		}
	}

	if ((generator == iterativeoverlap) || (generator == iterativeClusteredOverlap)) {
		dumpGraph(gp, g);
	}

	if (!outputDot) {
		/* terminate output stream */
		fprintf(out, "$\n");
	}

	/* global garbage collection */
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}	
