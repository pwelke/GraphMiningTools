#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "../graph.h"
#include "../searchTree.h"
#include "../loading.h"
#include "../listSpanningTrees.h"
#include "../listComponents.h"
#include "../upperBoundsForSpanningTrees.h"
#include "../connectedComponents.h"
#include "../cs_Tree.h"
#include "../wilsonsAlgorithm.h"

char DEBUG_INFO = 1;

typedef enum {
		wilson,
		listing,
		mix,
		cactus,
		bridgeForest,
		listOrSample
	} SamplingMethod;	



/**
 * Print --help message
 */
int printHelp() {
	FILE* helpFile = fopen("executables/treeSamplingHelp.txt", "r");
	if (helpFile != NULL) {
		int c = EOF;
		while ((c = fgetc(helpFile)) != EOF) {
			fputc(c, stdout);
		}
		fclose(helpFile);
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Could not read helpfile\n");
		return EXIT_FAILURE;
	}
}


/**
Sample a spanning tree from a cactus graph, given as a list of its biconnected components, uniformly at random.
To this end, we just need to remove a random edge from each cycle = block of the graph. **/
struct ShallowGraph* sampleSpanningTreeEdgesFromCactus(struct ShallowGraph* biconnectedComponents, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTree = getShallowGraph(sgp);
	struct ShallowGraph* idx;
	for (idx=biconnectedComponents; idx!=NULL; idx=idx->next) {
		if (idx->m == 1) {
			appendEdge(spanningTree, shallowCopyEdge(idx->edges, sgp->listPool));
		} else {
			int removalEdgeId = rand() % idx->m;
			struct VertexList* e;
			int i = 0;
			for (e=idx->edges; e!=NULL; e=e->next) {
				if (i != removalEdgeId) {
					appendEdge(spanningTree, shallowCopyEdge(e, sgp->listPool));
				}
				++i;
			}
		}
	}
	return spanningTree;
}


/**
Sample a spanning tree from a cactus graph, given as a list of its biconnected components, uniformly at random.
To this end, we just need to remove a random edge from each cycle = block of the graph. **/
struct Graph* sampleSpanningTreeFromCactus(struct Graph* original, struct ShallowGraph* biconnectedComponents, struct GraphPool* gp) {
	struct Graph* spanningTree = emptyGraph(original, gp);
	struct ShallowGraph* idx;
	for (idx=biconnectedComponents; idx!=NULL; idx=idx->next) {
		if (idx->m == 1) {
			addEdgeBetweenVertices(idx->edges->startPoint->number, idx->edges->endPoint->number, idx->edges->label, spanningTree, gp);
		} else {
			int removalEdgeId = rand() % idx->m;
			struct VertexList* e;
			int i = 0;
			for (e=idx->edges; e!=NULL; e=e->next) {
				if (i != removalEdgeId) {
					addEdgeBetweenVertices(e->startPoint->number, e->endPoint->number, e->label, spanningTree, gp);
				}
				++i;
			}
		}
	}
	return spanningTree;
}


/**
Take k random spanning trees of g using Wilsons algorithm and return them as a list.
*/
struct ShallowGraph* sampleSpanningTreesUsingWilson(struct Graph* g, int k, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL;
	int j;
	for (j=0; j<k; ++j) {
		struct ShallowGraph* spanningTree = randomSpanningTreeAsShallowGraph(g, sgp);
		spanningTree->next = spanningTrees;
		spanningTrees = spanningTree;
	}
	return spanningTrees;
}

/**
List all spanning trees of g and draw k of them uniformly at random, return these k spanning trees as a list. 
*/
struct ShallowGraph* sampleSpanningTreesUsingListing(struct Graph* g, int k, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL;
	struct ShallowGraph* trees = listSpanningTrees(g, sgp, gp);
	struct ShallowGraph** array;
	
	struct ShallowGraph* idx;
	int j;
	int nTrees = 0;

	/* find number of listed trees, put them in array */
	for (idx=trees; idx; idx=idx->next) ++nTrees;

	array = malloc(nTrees * sizeof(struct ShallowGraph*));
	j = 0;
	for (idx=trees; idx; idx=idx->next) {	
		array[j] = idx;
		++j;
	}

	/* sample k trees uniformly at random */
	for (j=0; j<k; ++j) {
		int rnd = rand() % nTrees;
		// can't just use the listed tree itself, as it might get selected more than once
		struct ShallowGraph* tree = cloneShallowGraph(array[rnd], sgp);
		tree->next = spanningTrees;
		spanningTrees = tree;
	}
	dumpShallowGraphCycle(sgp, trees);
	free(array);
	return spanningTrees;
}


/**
If there are expected to be less than threshold spanning trees, sample spanning trees using explicit listing, 
otherwise use wilsons algorithm.
*/
struct ShallowGraph* sampleSpanningTreesUsingMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	long upperBound = getGoodEstimate(g, sgp, gp);
	if ((upperBound < threshold) && (upperBound != -1)) {
		return sampleSpanningTreesUsingListing(g, k, gp, sgp);
	} else {
		return sampleSpanningTreesUsingWilson(g, k, sgp);
	}
}


/**
If g is a cactus graph, use a specialized method to sample spanning trees, 
otherwise use sampleSpanningTreesUsingMix.
*/
struct ShallowGraph* sampleSpanningTreesUsingCactusMix(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL;
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	int blockCount = 0;
	struct ShallowGraph* idx;
	for (idx=biconnectedComponents; idx!=NULL; idx=idx->next) {
		if (idx->m > 1) {
			++blockCount;
		}
	}
	/* if g is a cactus graph */
	if (g->n - 1 + blockCount == g->m) {
		int j;
		for (j=0; j<k; ++j) {	
			struct ShallowGraph* spanningTree = sampleSpanningTreeEdgesFromCactus(biconnectedComponents, sgp);
			spanningTree->next = spanningTrees;
			spanningTrees = spanningTree;
		}
	} else {
		// for speedup. this is sampleSpanningTreesUsingMix
		long upperBound = getGoodEstimatePrecomputedBlocks(g, biconnectedComponents, sgp, gp);
		if ((upperBound < threshold) && (upperBound != -1)) {
			spanningTrees = sampleSpanningTreesUsingListing(g, k, gp, sgp);
		} else {
			spanningTrees = sampleSpanningTreesUsingWilson(g, k, sgp);
		}
	}
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return spanningTrees;
}


/**
Return the list of trees in the bridge forest of g.
*/
struct ShallowGraph* listBridgeForest(struct Graph* g, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	// TODO replace this by a method that just creates the forest without listing cycles
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	struct ShallowGraph* bridgeTrees = getConnectedComponents(forest, sgp);
	struct ShallowGraph* tmp;
	// the resulting trees need to reference the original graph g
	for (tmp=bridgeTrees; tmp!=NULL; tmp=tmp->next) {
		rebaseShallowGraph(tmp, g);
	}
	/* garbage collection */
	dumpGraphList(gp, forest);
	return bridgeTrees;
}


/**
If there are expected to be less than threshold spanning trees, return a list containing all of them. 
Otherwise, sample k spanning trees using Wilsons algorithm. 
*/
struct ShallowGraph* listOrSampleSpanningTrees(struct Graph* g, int k, long int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* spanningTrees = NULL; 
	long upperBound = getGoodEstimate(g, sgp, gp);
	if ((upperBound < threshold) && (upperBound != -1)) {
		spanningTrees = listSpanningTrees(g, sgp, gp);	
	} else {
		spanningTrees = sampleSpanningTreesUsingWilson(g, k, sgp);
	}
	return spanningTrees;
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

	/* user input handling variables */
	long int threshold = 100;
	int k = 1;
	SamplingMethod samplingMethod = wilson;
	char unsafe = 0;

	/* i counts the number of graphs read */
	int i = 0;
	long int avgTrees = 0;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hs:k:t:o:u";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
			break;
		case 'k':
			if (sscanf(optarg, "%i", &k) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'u':
			unsafe = 1;
			break;
		case 't':
			if (sscanf(optarg, "%li", &threshold) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			} else {
				// negative encodes 'no restriction'
				if (threshold < 0) {
					threshold = LONG_MAX / 2;
				}
			}
			break;
		case 's':
			if (strcmp(optarg, "wilson") == 0) {
				samplingMethod = wilson;
				break;
			}
			if (strcmp(optarg, "listing") == 0) {
				samplingMethod = listing;
				break;
			}
			if (strcmp(optarg, "mix") == 0) {
				samplingMethod = mix;
				break;
			}
			if (strcmp(optarg, "cactus") == 0) {
				samplingMethod = cactus;
				break;
			}
			if (strcmp(optarg, "bridgeForest") == 0) {
				samplingMethod = bridgeForest;
				break;
			}
			if (strcmp(optarg, "listOrSample") == 0) {
				samplingMethod = listOrSample;
				break;
			}
			fprintf(stderr, "Unknown sampling method: %s\n", optarg);
			return EXIT_FAILURE;
			break;
		case '?':
			return EXIT_FAILURE;
			break;
		}
	}

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
	while ((g = iterateFile(&intLabel, &intLabel))) {
	
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n > 0) {
			if (unsafe || isConnected(g)) {
				struct Vertex* searchTree = getVertex(gp->vertexPool);
				struct ShallowGraph* sample = NULL;
				struct ShallowGraph* tree;

				switch (samplingMethod) {
				case wilson:
					sample = sampleSpanningTreesUsingWilson(g, k, sgp);
					break;
				case listing:
					sample = sampleSpanningTreesUsingListing(g, k, gp, sgp);
					break;
				case mix:
					sample = sampleSpanningTreesUsingMix(g, k, threshold, gp, sgp);
					break;
				case cactus:
					sample = sampleSpanningTreesUsingCactusMix(g, k, threshold, gp, sgp);
					break;
				case bridgeForest:
					sample = listBridgeForest(g, gp, sgp);
					break;
				case listOrSample:
					sample = listOrSampleSpanningTrees(g, k, threshold, gp, sgp);
					break;
				}

				for (tree=sample; tree!=NULL; tree=tree->next) {
					struct Graph* tmp = shallowGraphToGraph(tree, gp);
					struct ShallowGraph* cString = canonicalStringOfTree(tmp, sgp);
					addToSearchTree(searchTree, cString, gp, sgp);
					/* garbage collection */
					dumpGraph(gp, tmp);
				}

				/* output tree patterns represented as canonical strings */
				printf("# %i %i\n", g->number, searchTree->d);
				printStringsInSearchTree(searchTree, stdout, sgp);
				fflush(stdout);

				avgTrees += searchTree->number;
				dumpShallowGraphCycle(sgp, sample);
				dumpSearchTree(gp, searchTree);
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

	fprintf(stderr, "avgTrees = %f\n", avgTrees / (double)i);

	/* global garbage collection */
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}
