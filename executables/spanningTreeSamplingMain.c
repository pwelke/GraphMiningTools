#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>

#include "../graph.h"
#include "../searchTree.h"
#include "../loading.h"
#include "../listSpanningTrees.h"
#include "../listComponents.h"
#include "../upperBoundsForSpanningTrees.h"
#include "../subtreeIsomorphism.h"
#include "../graphPrinting.h"
#include "../treeCenter.h"
#include "../connectedComponents.h"
#include "../cs_Tree.h"
#include "../wilsonsAlgorithm.h"

char DEBUG_INFO = 1;


/**
 * Print --help message
 */
int printHelp() {
	FILE* helpFile = fopen("executables/spanningTreeSamplingHelp.txt", "r");
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
		// /* getGoodEstimate returns an upper bound on the number of spanning
		// trees in g, or -1 if there was an overflow of long ints while computing */
		// long upperBound = getGoodEstimate(g, sgp, gp);
		// if ((upperBound < depth) && (upperBound != -1)) {

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


struct ShallowGraph* sampleSpanningTreesUsingMix(struct Graph* g, int k, int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	long upperBound = getGoodEstimate(g, sgp, gp);
	if ((upperBound < threshold) && (upperBound != -1)) {
		return sampleSpanningTreesUsingListing(g, k, gp, sgp);
	} else {
		return sampleSpanningTreesUsingWilson(g, k, sgp);
	}
}

struct ShallowGraph* sampleSpanningTreesUsingCactusMix(struct Graph* g, int k, int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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

struct ShallowGraph* listOrSampleSpanningTrees(struct Graph* g, int k, int threshold, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
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
	if ((argc < 2) || (strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)) {
		printHelp();
		return EXIT_FAILURE;
	} else {

		/* create object pools */
		struct ListPool *lp = createListPool(10000);
		struct VertexPool *vp = createVertexPool(10000);
		struct ShallowGraphPool *sgp = createShallowGraphPool(1000, lp);
		struct GraphPool *gp = createGraphPool(100, vp, lp);

		/* pointer to the current graph which is returned by the input iterator */
		struct Graph* g = NULL;

		/* user input handling variables */
		char outputOption = 0;
		int param;
		long int threshold = 1000;
		int k = 1;

		/* i counts the number of graphs read */
		int i = 0;

		/* graph delimiter */
		int maxGraphs = -1;
		int minGraph = 0;

		long int avgTrees = 0;
		int cactusGraphs = 0;

		/* user input handling */
		for (param=2; param<argc; param+=2) {
			char known = 0;
			if ((strcmp(argv[param], "--help") == 0) || (strcmp(argv[param], "-h") == 0)) {
				printHelp();
				return EXIT_SUCCESS;
			}
			if (strcmp(argv[param], "-limit") == 0) {
				sscanf(argv[param+1], "%i", &maxGraphs);
				known = 1;
			}
			if (strcmp(argv[param], "-output") == 0) {
				outputOption = argv[param+1][0];
				known = 1;
			}
			if (strcmp(argv[param], "-bound") == 0) {
				sscanf(argv[param+1], "%li", &threshold);
				if (threshold < 0) {
					threshold = LONG_MAX / 2;
				}
				known = 1;
			}
			if (strcmp(argv[param], "-k") == 0) {
				sscanf(argv[param+1], "%i", &k);
				known = 1;
			}
			if (strcmp(argv[param], "-min") == 0) {
				sscanf(argv[param+1], "%i", &minGraph);
				known = 1;
			}
			if (!known) {
				fprintf(stderr, "Unknown parameter: %s\n",argv[param]);
				return(EXIT_FAILURE);
			}
		}

		if (outputOption == 0) {
			outputOption = 'w';
		}

		/* try to load a file */
		createFileIterator(argv[1], gp);

		/* iterate over all graphs in the database */
		while ((g = iterateFile(&intLabel, &intLabel))) {
		
			/* if there was an error reading some graph the returned n will be -1 */
			if (g->n > 0) {
				if (isConnected(g)) {
					struct Vertex* searchTree = getVertex(gp->vertexPool);
					struct ShallowGraph* sample = NULL;
					struct ShallowGraph* tree;

					switch (outputOption) {
					case 'w':
						sample = sampleSpanningTreesUsingWilson(g, k, sgp);
						break;
					case 'l':
						sample = sampleSpanningTreesUsingListing(g, k, gp, sgp);
						break;
					case 'p':
						sample = sampleSpanningTreesUsingMix(g, k, threshold, gp, sgp);
						break;
					case 'c':
						sample = sampleSpanningTreesUsingCactusMix(g, k, threshold, gp, sgp);
						break;
					case 'b':
						sample = listBridgeForest(g, gp, sgp);
						break;
					case 'x':
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
		fprintf(stderr, "#cactusGraphs = %i\n", cactusGraphs);

		/* global garbage collection */
		destroyFileIterator();
		freeGraphPool(gp);
		freeShallowGraphPool(sgp);
		freeListPool(lp);
		freeVertexPool(vp);

		return EXIT_SUCCESS;
	}
}
