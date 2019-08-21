#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>

#include "../graph.h"
#include "../loading.h"
#include "../listComponents.h"
#include "../listSpanningTrees.h"
#include "../outerplanar.h"
#include "../upperBoundsForSpanningTrees.h"
#include "../connectedComponents.h"
#include "../listCycles.h"
#include "../hp_cactus.h"
#include "../graphPrinting.h"
#include "../localEasiness.h"
#include "../sampleSubtrees.h"
#include "../localEasySubtreeIsomorphism.h"
#include "filter.h"


/**
 * Print --help message
 */
int printHelp() {
#include "../o/help/filterHelp.help"
	unsigned char* help = executables_filterHelp_txt;
	int len = executables_filterHelp_txt_len;
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

	/* pointer to the current graph which is returned by the input iterator */
	struct Graph* g = NULL;
	int i = 0;

	/* output */
	FILE* out = stdout;

	/* user set variables to specify what needs to be done */
	Filter filter = count;
	Comparator comparator = pass;
	OutputOption oOption = graph;
	int value = -1;
	/* can be set via -a. Used e.g. by spanningTreeListing filter, and randomSample*/
	int additionalParameter = 100;
	int randomSeed = time(NULL);

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hf:c:v:o:a:r:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
		case 'f':
			/* labels */
			if (strcmp(optarg, "label") == 0) {
				filter = label;
				break;
			}
			/* counting */
			if (strcmp(optarg, "graphName") == 0) {
				filter = graphName;
				break;
			}
			if (strcmp(optarg, "count") == 0) {
				filter = count;
				break;
			}
			if (strcmp(optarg, "connected") == 0) {
				filter = connected;
				break;
			}
			if (strcmp(optarg, "outerplanar") == 0) {
				filter = outerplanar;
				break;
			}
			if (strcmp(optarg, "tree") == 0) {
				filter = tree;
				break;
			}
			if (strcmp(optarg, "cactus") == 0) {
				filter = cactus;
				break;
			}
			if (strcmp(optarg, "path") == 0) {
				filter = path;
				break;
			}
			if (strcmp(optarg, "traceableCactus") == 0) {
				filter = traceableCactus;
				break;
			}
			if (strcmp(optarg, "weaklyTraceable") == 0) {
				filter = weaklyTraceable;
				break;
			}
			if (strcmp(optarg, "spanningTreeEstimate") == 0) {
				filter = spanningTreeEstimate;
				break;
			}
			if (strcmp(optarg, "spanningTreeListing") == 0) {
				filter = spanningTreeListing;
				break;
			}
			if (strcmp(optarg, "nonisomorphicSpanningTrees") == 0) {
				filter = nonisomorphicSpanningTrees;
				break;
			}
			if (strcmp(optarg, "sampledSpanningTreesFiltered") == 0) {
				filter = sampledSpanningTreesFiltered;
				break;
			}
			if (strcmp(optarg, "nonisomorphicSampledSpanningTrees") == 0) {
				filter = nonisomorphicSampledSpanningTrees;
				break;
			}
			if (strcmp(optarg, "nonisomorphicLocallySampledSpanningTrees") == 0) {
				filter = nonisomorphicLocallySampledSpanningTrees;
				break;
			}
			if (strcmp(optarg, "nonisomorphicLocallySampledSpanningTreesFiltered") == 0) {
				filter = nonisomorphicLocallySampledSpanningTreesFiltered;
				break;
			}
			if (strcmp(optarg, "locallySampledSpanningTrees") == 0) {
				filter = locallySampledSpanningTrees;
				break;
			}
			if (strcmp(optarg, "locallySampledSpanningTreesFiltered") == 0) {
				filter = locallySampledSpanningTreesFiltered;
				break;
			}
			if (strcmp(optarg, "nonisomorphicSampledSpanningTreesNormalized") == 0) {
				filter = nonisomorphicSampledSpanningTreesNormalized;
				break;
			}
			if (strcmp(optarg, "nonisomorphicLocallySampledSpanningTreesNormalized") == 0) {
				filter = nonisomorphicLocallySampledSpanningTreesNormalized;
				break;
			}
			if (strcmp(optarg, "nonisomorphicLocallySampledSpanningTreesFilteredNormalized") == 0) {
				filter = nonisomorphicLocallySampledSpanningTreesFilteredNormalized;
				break;
			}
			if (strcmp(optarg, "locallySampledSpanningTreesNormalized") == 0) {
				filter = locallySampledSpanningTreesNormalized;
				break;
			}
			if (strcmp(optarg, "locallySampledSpanningTreesFilteredNormalized") == 0) {
				filter = locallySampledSpanningTreesFilteredNormalized;
				break;
			}
			if (strcmp(optarg, "maxBlocksPerComponent") == 0) {
				filter = maxBlocksPerComponent;
				break;
			}
			if (strcmp(optarg, "numberOfBlocks") == 0) {
				filter = numberOfBlocks;
				break;
			}
			if (strcmp(optarg, "numberOfBridges") == 0) {
				filter = numberOfBridges;
				break;
			}
			if (strcmp(optarg, "numberOfBridgeTrees") == 0) {
				filter = numberOfBridgeTrees;
				break;
			}
			if (strcmp(optarg, "numberOfSimpleCycles") == 0) {
				filter = numberOfSimpleCycles;
				break;
			}
			if (strcmp(optarg, "numberOfNonIsoCycles") == 0) {
				filter = numberOfNonIsoCycles;
				break;
			}
			if (strcmp(optarg, "numberOfVertices") == 0) {
				filter = numberOfVertices;
				break;
			}
			if (strcmp(optarg, "numberOfBiconnectedComponents") == 0) {
				filter = numberOfBiconnectedComponents;
				break;
			}
			if (strcmp(optarg, "numberOfConnectedComponents") == 0) {
				filter = numberOfConnectedComponents;
				break;
			}
			if (strcmp(optarg, "numberOfEdges") == 0) {
				filter = numberOfEdges;
				break;
			}
			if (strcmp(optarg, "maxCycleDegree") == 0) {
				filter = maxCycleDegree;
				break;
			}
			if (strcmp(optarg, "minCycleDegree") == 0) {
				filter = minCycleDegree;
				break;
			}
			if (strcmp(optarg, "minLocalEasiness") == 0) {
				filter = minLocalEasiness;
				break;
			}
			if (strcmp(optarg, "maxLocalEasiness") == 0) {
				filter = maxLocalEasiness;
				break;
			}
			if (strcmp(optarg, "maxDegree") == 0) {
				filter = maxDegree;
				break;
			}
			if (strcmp(optarg, "minDegree") == 0) {
				filter = minDegree;
				break;
			}
			if (strcmp(optarg, "randomSample") == 0) {
				filter = randomSample;
				break;
			}
			fprintf(stderr, "Unknown filter: %s\n", optarg);
			return EXIT_FAILURE;
			break;
		case 'c':
			if (strcmp(optarg, "==") == 0) {
				comparator = eq;
				break;
			}
			if (strcmp(optarg, "!=") == 0) {
				comparator = neq;
				break;
			}
			if (strcmp(optarg, "<=") == 0) {
				comparator = leq;
				break;
			}
			if (strcmp(optarg, ">=") == 0) {
				comparator = geq;
				break;
			}
			if (strcmp(optarg, "<") == 0) {
				comparator = less;
				break;
			}
			if (strcmp(optarg, ">") == 0) {
				comparator = greater;
				break;
			}
			fprintf(stderr, "Unknown comparator: %s\n", optarg);
			return EXIT_FAILURE;
			break;
		case 'v':
			if (sscanf(optarg, "%i", &value) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'a':
			if (sscanf(optarg, "%i", &additionalParameter) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'r':
			if (sscanf(optarg, "%i", &randomSeed) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'o':
			if ((strcmp(optarg, "graph") == 0) || (strcmp(optarg, "g") == 0)) {
				oOption = graph;
				break;
			}
			if ((strcmp(optarg, "idAndValue") == 0) || (strcmp(optarg, "iv") == 0)) {
				oOption = idAndValue;
				break;
			}
			if ((strcmp(optarg, "id") == 0) || (strcmp(optarg, "i") == 0)) {
				oOption = id;
				break;
			}
			if ((strcmp(optarg, "value") == 0) || (strcmp(optarg, "v") == 0)) {
				oOption = onlyValue;
				break;
			}
			if ((strcmp(optarg, "print") == 0) || (strcmp(optarg, "p") == 0)) {
				oOption = printVerbose;
				break;
			}
			fprintf(stderr, "Unknown output option: %s\n", optarg);
			return EXIT_FAILURE;
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
	gp = createGraphPool(100, vp, lp);

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
	while ((g = iterateFile())) {
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n != -1) {
			
			processGraph(i, g, filter, comparator, value, additionalParameter, out, oOption, sgp, gp);

			/***** do not alter ****/

			++i;
			/* garbage collection */
			dumpGraph(gp, g);

		} else {
			/* TODO should be handled by dumpgraph */
			free(g);
		}
	}

	/* terminate output stream, if we write graphs */
	if (oOption == graph) {
		fprintf(out, "$\n");
	}

	/* global garbage collection */
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}	

void processGraph(int i, struct Graph* g, Filter filter, Comparator comparator, int value, int additionalParameter, FILE* out, OutputOption oOption, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	int measure = -1;
	switch (filter) {

	/* counts */ 
	case count:
		// TODO break condition for <= ==
		measure = i;
		break;
	case graphName:
		measure = g->number;
		break;
	case randomSample:
		measure = rand() % 1000;
		break;
	/* labels */
	case label:
		measure = g->activity;
		break;

	/* boolean properties */
	case connected:
		measure = isConnected(g);
		break;
	case outerplanar:
		// isMaximalOuterplanar alters g. do not attempt to do something with g after this.
		measure = isOuterplanarGraph(g, sgp, gp);
		break;
	case tree:
		measure = isTree(g);
		break;
	case cactus:
		measure = isCactus(g, sgp);
		break;
	case path:
		measure = isPath(g);
		break;
	case traceableCactus:
		measure = isTraceableCactus(g, sgp);
		break;
	case weaklyTraceable:
		measure = isWeaklyTraceable(g, sgp);
		break;

	/* numeric properties */
	case spanningTreeEstimate:
		measure = getGoodEstimate(g, sgp, gp);
		break;
	case spanningTreeListing:
		measure = countSpanningTrees(g, additionalParameter, sgp, gp);
		break;
	case nonisomorphicSpanningTrees:
		measure = countNonisomorphicSpanningTrees(g, gp, sgp);
		break;
	case nonisomorphicSampledSpanningTrees:
		measure = getNumberOfNonisomorphicSpanningForestComponentsForKSamples(g, additionalParameter, gp, sgp);
		break;
	case sampledSpanningTreesFiltered:
		measure = getNumberOfDifferentSpanningForestComponentsForKSamples(g, additionalParameter, gp, sgp);
		break;
	case nonisomorphicLocallySampledSpanningTrees:
		measure = getNumberOfNonisomorphicSpanningTreesObtainedByLocalEasySampling(g, additionalParameter, gp, sgp);
		break;
	case nonisomorphicLocallySampledSpanningTreesFiltered:
		measure = getNumberOfNonisomorphicSpanningTreesObtainedByLocalEasySamplingWithFiltering(g, additionalParameter, gp, sgp);
		break;
	case locallySampledSpanningTrees:
		measure = getNumberOfSpanningTreesObtainedByLocalEasySampling(g, additionalParameter, gp, sgp);
		break;
	case locallySampledSpanningTreesFiltered:
		measure = getNumberOfSpanningTreesObtainedByLocalEasySamplingWithFiltering(g, additionalParameter, gp, sgp);
		break;

	case nonisomorphicSampledSpanningTreesNormalized:
		measure = getNumberOfNonisomorphicSpanningForestComponentsForKSamples(g, additionalParameter, gp, sgp);
		measure /= getAndMarkConnectedComponents(g);
		break;
	case nonisomorphicLocallySampledSpanningTreesNormalized:
		measure = getNumberOfNonisomorphicSpanningTreesObtainedByLocalEasySampling(g, additionalParameter, gp, sgp);
		measure /= getAndMarkConnectedComponents(g);
		break;
	case nonisomorphicLocallySampledSpanningTreesFilteredNormalized:
		measure = getNumberOfNonisomorphicSpanningTreesObtainedByLocalEasySamplingWithFiltering(g, additionalParameter, gp, sgp);
		measure /= getAndMarkConnectedComponents(g);
		break;
	case locallySampledSpanningTreesNormalized:
		measure = getNumberOfSpanningTreesObtainedByLocalEasySampling(g, additionalParameter, gp, sgp);
		measure /= getAndMarkConnectedComponents(g);
		break;
	case locallySampledSpanningTreesFilteredNormalized:
		measure = getNumberOfSpanningTreesObtainedByLocalEasySamplingWithFiltering(g, additionalParameter, gp, sgp);
		measure /= getAndMarkConnectedComponents(g);
		break;

	case numberOfBlocks:
		measure = getNumberOfBlocks(g, sgp);			
		break;
	case maxBlocksPerComponent:
		measure = getMaxNumberOfBlocksPerComponent(g, gp, sgp);
		break;
	case numberOfBridges:
		measure = getNumberOfBridges(g, sgp);			
		break;
	case numberOfBridgeTrees:
		measure = getNumberOfBridgeTrees(g, sgp, gp);			
		break;
	case numberOfVertices:
		measure = g->n;
		break;
	case numberOfEdges:
		measure = g->m;
		break;
	case numberOfNonIsoCycles:
		measure = getNumberOfNonIsoCycles(g, sgp, gp);
		break;
	case numberOfSimpleCycles:
		measure = getNumberOfSimpleCycles(g, sgp, gp);
		break;		
	case numberOfBiconnectedComponents:
		measure = getNumberOfBiconnectedComponents(g, sgp);
		break;
	case numberOfConnectedComponents:
		measure = getAndMarkConnectedComponents(g);
		break;
	case maxDegree:
		measure = getMaxDegree(g);
		break;
	case minDegree:
		measure = getMinDegree(g);
		break;
	case maxCycleDegree:
		measure = getMaxCycleDegree(g, sgp);
		break;
	case minCycleDegree:
		measure = getMinCycleDegree(g, sgp);
		break;
	case maxLocalEasiness:
		measure = getMaxLocalEasiness(g, additionalParameter, gp, sgp);
		break;
	case minLocalEasiness:
		measure = getMinLocalEasiness(g, additionalParameter, gp, sgp);
		break;
	}

	if (conditionHolds(measure, value, comparator)) {
		output(g, measure, oOption, out);
	}
}


/**
Check if a condition of the form measure comparator threshold holds.
E.g. measure <= threshold.
*/
char conditionHolds(int measure, int threshold, Comparator comparator) {
	switch (comparator) {
	case leq:
		return measure <= threshold;
		break;
	case eq:
		return measure == threshold;
		break;
	case geq:
		return measure >= threshold;
		break;
	case neq:
		return measure != threshold;
		break;
	case less:
		return measure < threshold;
		break;
	case greater:
		return measure > threshold;
		break;
	case pass:
		return 1;
		break;
	}
	// should not happen
	return 0;
}


void output(struct Graph* g, int measure, OutputOption option, FILE* out) {
	switch (option) {
	case graph:
		writeCurrentGraph(out);
		break;
	case onlyValue:
		fprintf(out, "%i\n", measure);
		break;
	case idAndValue:
		fprintf(out, "%i %i\n", g->number, measure);
		break;
	case id:
		fprintf(out, "%i\n", g->number);
		break;
	case printVerbose:
		printGraph(g);
	}
} 

