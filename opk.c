#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "dfs.h"
#include "searchTree.h"
#include "canonicalString.h"
#include "outerplanar.h"
#include "treeKernels.h"
#include "opk.h"




/**
 * Implementation of the feature extraction phase of the Outerplanar BBTree Kernel
 * described in the documentation
 */
void outerplanarKernel(struct Graph *g, int depth, struct ShallowGraphPool *sgp, struct GraphPool *gp,
		char outputOptions, struct Vertex* globalTreeSet, struct compInfo** results, int* resSize) {

	struct ShallowGraph* biconnectedComponents = findBiconnectedComponents(g, sgp);
	struct BBTree* bbTree = createBlockAndBridgeTree(biconnectedComponents, g, gp, sgp);

	/* if the depth of dfs is not set to a constant, it will be n */
	if (depth == -1) {
		depth = g->n;
	}

	if (bbTree) {

		/* start a tree kernel on both bridge and block vertices */
		struct ShallowGraph* trees1 = bfsSubtreeEnumeration(bbTree->tree, depth, sgp);
		struct Vertex* searchTree = buildSearchTree(trees1, gp, sgp);

		int pos = 0;

		if (bbTree->blocks) {
			struct ShallowGraph* trees2 = bfsSubtreeEnumeration(bbTree->blocks, depth, sgp);
			struct Vertex* tmpSearchTree = buildSearchTree(trees2, gp, sgp);

			/* adjust storage size if necessary */
			if (searchTree->number + tmpSearchTree->number > *resSize) {
				if (*results) {
					free(*results);
				}
				*results = getResultVector(searchTree->number + tmpSearchTree->number);
				*resSize = searchTree->number + tmpSearchTree->number;
			}
			shallowMergeSearchTrees(searchTree, tmpSearchTree, 1, *results, &pos, searchTree, 0, gp);

			/* garbage collection */
			dumpSearchTree(gp, tmpSearchTree);

		}



		/* adjust storage size if necessary */
		if (searchTree->number > *resSize) {
			if (*results) {
				free(*results);
			}
			*results = getResultVector(searchTree->number);
			*resSize = searchTree->number;
		}



		pos = 0;
		/* add elements to global search trees to obtain mapping from strings to integers */
		mergeSearchTrees(globalTreeSet, searchTree, 1, *results, &pos, globalTreeSet, 0, gp);


		/* sort the output elements by increasing id */
		qsort(*results, pos, sizeof(struct compInfo), &compInfoComparison);

		/* output options */
		switch (outputOptions) {
		int i;
		int n;

		case 'a':
			/* "all" output the feature vector for the current graph in the format specified
			 * by SVMlight g->activity is assumed to be either 0, 1 or -1 to be compliant with
			 * the specs of SVMlight.
			 */

			/* print the id and label of the graph */
			printf("%i", g->activity);

			/* print key:value pairs */
			for (i=0; i<pos; ++i) {
				printf(" %i:%i", (*results)[i].id, (*results)[i].count);
			}
			printf("\n");
			break;
		case 'o':
			printf("%i %i\n", g->number, (bbTree) ? 1 : 0);
			break;
		case 'c':
			/* returns the number of cycles found in the graph */
			if (searchTree)
				printf("%i %i\n", g->number, searchTree->number / 2);
			else
				printf("%i 0\n", g->number);
			break;
		case 'v':
			/* returns the number of vertices in the graph */
			printf("%i %i\n", g->number, g->n);
			break;
		case 'e':
			/* returns the number of edges in the graph */
			printf("%i %i\n", g->number, g->m);
			break;
		case 'b':
			/* returns the number of vertices in the bbtree */
			n = 0;
			if (bbTree) {
				if (bbTree->blocks) {
					n += bbTree->blocks->n;
				}
				if (bbTree->tree) {
					for (i=0; i<bbTree->tree->n; ++i) {
						if (bbTree->tree->vertices[i]) {
							++n;
						}
					}
				}
			} else {
				n = -1;
			}
			printf("%i %i\n", g->number, n);
			break;
		case 'd':
			printf("%i", g->number);
			if (bbTree->blocks) {
				for (i=0; i<bbTree->blocks->n; ++i) {
					struct Graph* tmpGraph = shallowGraphToGraph(bbTree->blockComponents[i], gp);
					printf(" %i", tmpGraph->m - tmpGraph->n);
					dumpGraph(gp, tmpGraph);
				}
			} else {
				printf(" -1");
			}
			printf("\n");
			break;
		}


		/* prelim garbage collection */
		dumpSearchTree(gp, searchTree);
		dumpBBTree(gp, sgp, bbTree);
	}
}

/**
 * Implementation of the feature extraction phase of the Outerplanar BBTree Kernel
 * described in the documentation
 */
void freeOuterplanarKernel(struct Graph *g, int depth, struct ShallowGraphPool *sgp, struct GraphPool *gp,
		char outputOptions, struct Vertex* globalTreeSet, struct compInfo** results, int* resSize) {

	struct ShallowGraph* biconnectedComponents = findBiconnectedComponents(g, sgp);
	struct BBTree* bbTree = createBlockAndBridgeTree(biconnectedComponents, g, gp, sgp);

	/* if the depth of dfs is not set to a constant, it will be n */
	if (depth == -1) {
		depth = g->n;
	}

	if (bbTree) {


		/* start a tree kernel on both bridge and block vertices */
		struct ShallowGraph* trees1 = bfsSubtreeEnumeration(bbTree->tree, depth, sgp);
		struct Vertex* searchTree = buildSearchTree(trees1, gp, sgp);

		int pos = 0;

		if (bbTree->blocks) {
			struct ShallowGraph* trees2 = bfsFreeSubtreeEnumeration(bbTree->blocks, depth, sgp);
			struct Vertex* tmpSearchTree = buildSearchTree(trees2, gp, sgp);

			/* adjust storage size if necessary */
			if (searchTree->number + tmpSearchTree->number > *resSize) {
				if (*results) {
					free(*results);
				}
				*results = getResultVector(searchTree->number + tmpSearchTree->number);
				*resSize = searchTree->number + tmpSearchTree->number;
			}
			shallowMergeSearchTrees(searchTree, tmpSearchTree, 1, *results, &pos, searchTree, 0, gp);

			/* garbage collection */
			dumpSearchTree(gp, tmpSearchTree);

		}



		/* adjust storage size if necessary */
		if (searchTree->number > *resSize) {
			if (*results) {
				free(*results);
			}
			*results = getResultVector(searchTree->number);
			*resSize = searchTree->number;
		}



		pos = 0;
		/* add elements to global search trees to obtain mapping from strings to integers */
		mergeSearchTrees(globalTreeSet, searchTree, 1, *results, &pos, globalTreeSet, 0, gp);


		/* sort the output elements by increasing id */
		qsort(*results, pos, sizeof(struct compInfo), &compInfoComparison);

		/* output options */
		switch (outputOptions) {
		int i;
		int n;

		case 'a':
			/* "all" output the feature vector for the current graph in the format specified
			 * by SVMlight g->activity is assumed to be either 0, 1 or -1 to be compliant with
			 * the specs of SVMlight.
			 */

			/* print the id and label of the graph */
			printf("%i", g->activity);

			/* print key:value pairs */
			for (i=0; i<pos; ++i) {
				printf(" %i:%i", (*results)[i].id, (*results)[i].count);
			}
			printf("\n");
			break;
		case 'o':
			printf("%i %i\n", g->number, (bbTree) ? 1 : 0);
			break;
		case 'c':
			/* returns the number of cycles found in the graph */
			if (searchTree)
				printf("%i %i\n", g->number, searchTree->number / 2);
			else
				printf("%i 0\n", g->number);
			break;
		case 'v':
			/* returns the number of vertices in the graph */
			printf("%i %i\n", g->number, g->n);
			break;
		case 'e':
			/* returns the number of edges in the graph */
			printf("%i %i\n", g->number, g->m);
			break;
		case 'b':
			/* returns the number of vertices in the bbtree */
			n = 0;
			if (bbTree) {
				if (bbTree->blocks) {
					n += bbTree->blocks->n;
				}
				if (bbTree->tree) {
					for (i=0; i<bbTree->tree->n; ++i) {
						if (bbTree->tree->vertices[i]) {
							++n;
						}
					}
				}
			} else {
				n = -1;
			}
			printf("%i %i\n", g->number, n);
			break;
		case 'd':
			printf("%i", g->number);
			if (bbTree->blocks) {
				for (i=0; i<bbTree->blocks->n; ++i) {
					struct Graph* tmpGraph = shallowGraphToGraph(bbTree->blockComponents[i], gp);
					printf(" %i", tmpGraph->m - tmpGraph->n);
					dumpGraph(gp, tmpGraph);
				}
			} else {
				printf(" -1");
			}
			printf("\n");
			break;
		}


		/* prelim garbage collection */
		dumpSearchTree(gp, searchTree);
		dumpBBTree(gp, sgp, bbTree);
	}
}



