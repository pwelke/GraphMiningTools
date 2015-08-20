#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>

#include "../graph.h"
#include "../loading.h"
#include "../searchTree.h"
#include "../cs_Tree.h"
#include "../graphPrinting.h"
#include "../connectedComponents.h"
#include "../sampleSubtrees.h"
#include "treeSamplingMain.h"

/**
 * Print --help message
 */
static int printHelp() {
#include "treeSamplingHelp.help"
	unsigned char* help = executables_treeSamplingHelp_txt;
	int len = executables_treeSamplingHelp_txt_len;
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

	/* user input handling variables */
	long int threshold = 100;
	int k = 1;
	SamplingMethod samplingMethod = wilson;
	char unsafe = 0;
	char verbosity = 0;
	char processDisconnectedGraphs = 0;
	OutputMethod outputMethod = cs;

	/* i counts the number of graphs read */
	int i = 0;
	/* processedGraphs is the number of graphs that are considered (might be less, if some graphs are not connected) */
	int processedGraphs = 0;
	long int avgTrees = 0;
	/* set random seed */
	srand(time(NULL));

	/* parse command line arguments */
	int arg;
	int seed;
	const char* validArgs = "hs:k:t:o:ur:vd";
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
		case 'd':
			processDisconnectedGraphs = 1;
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
			if (strcmp(optarg, "kruskal") == 0) {
				samplingMethod = kruskal;
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
			if (strcmp(optarg, "partialListing") == 0) {
				samplingMethod = partialListing;
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
		case 'r':
			if (sscanf(optarg, "%i", &seed) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			} else {
				srand(seed);
			}
			break;
		case 'o':
			if (strcmp(optarg, "canonicalString") == 0) {
				outputMethod = cs;
				break;
			}
			if ((strcmp(optarg, "forest") == 0) || (strcmp(optarg, "graph") == 0)) {
				outputMethod = fo;
				break;
			}
			if (strcmp(optarg, "tree") == 0) {
				outputMethod = tr;
				break;
			}
			fprintf(stderr, "Unknown output method: %s\n", optarg);
			return EXIT_FAILURE;
			break;
		case 'v': 
			verbosity = 1;
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
	while ((g = iterateFile())) {
	
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n > 0) {
			if (unsafe || isConnected(g)) {
				struct Vertex* searchTree = getVertex(gp->vertexPool);
				struct ShallowGraph* sample = NULL;
				struct ShallowGraph* tree;

				if (!processDisconnectedGraphs) {
					switch (samplingMethod) {
					case wilson:
						sample = sampleSpanningTreesUsingWilson(g, k, sgp);
						break;
					case kruskal:
						sample = sampleSpanningTreesUsingKruskal(g, k, gp, sgp);
						break;
					case listing:
						sample = sampleSpanningTreesUsingListing(g, k, gp, sgp);
						break;
					case mix:
						sample = sampleSpanningTreesUsingMix(g, k, threshold, gp, sgp);
						break;
					case partialListing:
						sample = sampleSpanningTreesUsingPartialListingMix(g, k, threshold, gp, sgp);
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
				} else {
					switch (samplingMethod) {
					case wilson:
						sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingWilson,
							g, k, threshold, gp, sgp);
						break;
					case kruskal:
						// sample = sampleSpanningTreesUsingKruskal(g, k, gp, sgp);
						sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingKruskal,
							g, k, threshold, gp, sgp);
						break;
					case listing:
						// sample = sampleSpanningTreesUsingListing(g, k, gp, sgp);
						sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingListing,
							g, k, threshold, gp, sgp);
						break;
					case mix:
						// sample = sampleSpanningTreesUsingMix(g, k, threshold, gp, sgp);
						sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingMix,
							g, k, threshold, gp, sgp);
						break;
					case partialListing:
						// sample = sampleSpanningTreesUsingPartialListingMix(g, k, threshold, gp, sgp);
						sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingPartialListingMix,
							g, k, threshold, gp, sgp);
					case cactus:
						// sample = sampleSpanningTreesUsingCactusMix(g, k, threshold, gp, sgp);
						sample = runForEachConnectedComponent(&xsampleSpanningTreesUsingCactusMix,
							g, k, threshold, gp, sgp);
						break;
					case bridgeForest:
						// sample = listBridgeForest(g, gp, sgp);
						sample = runForEachConnectedComponent(&xlistBridgeForest,
							g, k, threshold, gp, sgp);
						break;
					case listOrSample:
						// sample = listOrSampleSpanningTrees(g, k, threshold, gp, sgp);
						sample = runForEachConnectedComponent(&xlistOrSampleSpanningTrees,
							g, k, threshold, gp, sgp);
						break;
					}
				}

				for (tree=sample; tree!=NULL; tree=tree->next) {
					if (tree->m != 0) {
						struct Graph* tmp = shallowGraphToGraph(tree, gp);
						struct ShallowGraph* cString = canonicalStringOfTree(tmp, sgp);
						addToSearchTree(searchTree, cString, gp, sgp);
						/* garbage collection */
						dumpGraph(gp, tmp);
					} else {
						// in the case of bridge forests or singleton graphs, we might get a tree without an edge.
						// sinlgeton graphs are not supported, yet.
						if (samplingMethod == bridgeForest) {
							struct ShallowGraph* cString = getShallowGraph(sgp);
							struct VertexList* e = getVertexList(sgp->listPool);
							e->label = g->vertices[tree->data]->label;
							appendEdge(cString, e);
							addToSearchTree(searchTree, cString, gp, sgp);
						}
					}
				}

				switch (outputMethod) {
				struct ShallowGraph* strings;
				struct ShallowGraph* string;
				struct Graph* forest;
				struct Vertex* rootNode;
				struct Graph* rootGraph;
				case cs:
					/* output tree patterns represented as canonical strings */
					printf("# %i %i\n", g->number, searchTree->d);
					printStringsInSearchTree(searchTree, stdout, sgp);
					fflush(stdout);
					break;
				case fo:
					/* output tree patterns as forest un standard format */
					forest = NULL;
					strings = listStringsInSearchTree(searchTree, sgp);
					for (string=strings; string!=NULL; string=string->next) {
						struct Graph* tmp;
						tmp = treeCanonicalString2Graph(string, gp);
						tmp->next = forest;
						forest = tmp;
					}
					forest = mergeGraphs(forest, gp);
					forest->number = g->number;
					forest->activity = g->activity;
					printGraphAidsFormat(forest, stdout);
					dumpGraph(gp, forest);
					dumpShallowGraphCycle(sgp, strings);
					break;
				case tr:
					/* output tree patterns as single tree in standard format by adding a new vertex with unique label */
					forest = createGraph(1, gp);
					rootGraph = forest;
					rootNode = forest->vertices[0];
					rootNode->label = intLabel(g->number);
					rootNode->isStringMaster = 1;

					strings = listStringsInSearchTree(searchTree, sgp);
					for (string=strings; string!=NULL; string=string->next) {
						struct VertexList* e = getVertexList(gp->listPool);
						struct Graph* tmp;
						tmp = treeCanonicalString2Graph(string, gp);

						/* evil: add edge between vertices that do not belong to the same graph, yet. */
						rootGraph->m += 1;
						e->startPoint = rootNode;
						e->endPoint = tmp->vertices[0];
						e->label = rootNode->label;
						addEdge(rootNode, e);
						addEdge(tmp->vertices[0], inverseEdge(e, gp->listPool));

						tmp->next = forest;
						forest = tmp;
					}
					forest = mergeGraphs(forest, gp);
					forest->number = g->number;
					forest->activity = g->activity;
					printGraphAidsFormat(forest, stdout);
					dumpGraph(gp, forest);
					dumpShallowGraphCycle(sgp, strings);
					break;
				}

				avgTrees += searchTree->number;
				dumpShallowGraphCycle(sgp, sample);
				dumpSearchTree(gp, searchTree);

				++processedGraphs;
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

	/* if output is standard graph db, terminate it with dollar sign */
	if ((outputMethod == tr)  || (outputMethod == fo)) {
		fprintf(stdout, "$\n");
	}

	if (verbosity) {
		fprintf(stderr, "avgTrees = %f\n", avgTrees / (double)processedGraphs);
	}

	/* global garbage collection */
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}
