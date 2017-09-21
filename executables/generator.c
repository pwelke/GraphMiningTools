#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>
#include <math.h>

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

//struct Graph* BarabasiAlbert(int n, struct GraphPool* gp) {
//	struct Graph* g = createGraph(n, gp);
//	addEdgeBetweenVertices(0, 1, NULL, g, gp);
//	g->vertices[0]->d = 1;
//	g->vertices[1]->d = 1;
//	for (int v=2; v<n; ++v) {
//		// TODO
//	}
//}

static double euclideanDistance(int vx, int vy, int wx, int wy) {
	double xdiff = ((double)abs(vx - wx)) / RAND_MAX;
	double ydiff = ((double)abs(vy - wy)) / RAND_MAX;
	return sqrt(xdiff * xdiff + ydiff * ydiff);
}

static double euclideanDistanceWrap(int v, int w, struct Graph* g) {
	return euclideanDistance(g->vertices[v]->d, g->vertices[v]->lowPoint, g->vertices[w]->d, g->vertices[w]->lowPoint);
}



struct Graph* randomOverlapGraph(int n, double d, struct GraphPool* gp) {
	struct Graph* g = createGraph(n, gp);

	// every vertex is a two-dimensional point
	for (int v=0; v<n; ++v) {
		g->vertices[v]->d = rand();
		g->vertices[v]->lowPoint = rand();
		g->vertices[v]->label = intLabel(1);
	}

	for (int v=0; v<n; ++v) {
		for (int w=v+1; w<n; ++w) {
			if (euclideanDistanceWrap(v, w, g) < d) {
				addEdgeBetweenVertices(v, w, intLabel(1), g, gp);
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

typedef enum {
	overlap, erdosrenyi, barabasialbert
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
	double edgeProbability = 0.5;
	double edgeMultiplicity = -1;
	int randomSeed = time(NULL);
	Generator generator = erdosrenyi;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hp:s:a:b:c:d:N:m:x:";
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
		case 'm':
			if (sscanf(optarg, "%lf", &edgeMultiplicity) != 1) {
				fprintf(stderr, "value must be double, is: %s\n", optarg);
				return EXIT_FAILURE;
			} 
			break;
		case 'x':
			if (strcmp(optarg, "overlap") == 0) {
				generator = overlap;
				break;
			}
			if (strcmp(optarg, "erdosrenyi") == 0) {
				generator = erdosrenyi;
				break;
			}
			if (strcmp(optarg, "barabasialbert") == 0) {
				generator = barabasialbert;
				break;
			}
			fprintf(stderr, "Unknown random generator: %s\n", optarg);
			return EXIT_FAILURE;
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
		int n = rand() % (upperBoundVertices - lowerBoundVertices) + lowerBoundVertices;
		/* calculate p, if m option was given */
		if ((edgeMultiplicity > 0) && (n > 1)) {
			edgeProbability = ( 2.0 * edgeMultiplicity ) / (n - 1.0);
		}
		switch (generator) {
		case erdosrenyi:
			g = erdosRenyiWithLabels(n, edgeProbability, nVertexLabels, nEdgeLabels, gp);
			break;
		case overlap:
			g = randomOverlapGraph(n, edgeProbability, gp);
		}
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
