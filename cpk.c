#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "listComponents.h"
#include "listCycles.h"
#include "cs_Tree.h"
#include "cs_Cycle.h"
#include "loading.h"
#include "cpk.h"


/**
 * This is the actual main function that computes a feature vector from the graph g
 *  does not dump g */
int CyclicPatternKernel(struct Graph *g, struct ShallowGraphPool *sgp, struct GraphPool *gp,
		char outputOptions, struct Vertex* globalTreeSet, struct Vertex* globalCycleSet, struct compInfo** results, int* resSize) {
	struct Graph* tmp;
	struct Graph* idx;
	int numCycles;
	int pos = 0;

	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	/* TODO refactor */
	struct Graph* biconnectedComponents = forest->next;

	/* list tree patterns */
	struct ShallowGraph* treePatterns = getTreePatterns(forest, sgp);

	/* create search tree structure */
	struct Vertex* treePatternSearchTree = buildSearchTree(treePatterns, gp, sgp);
	int numTrees = treePatternSearchTree->number;

	/* list all cycles */
	struct ShallowGraph* simpleCycles = NULL;
	struct ShallowGraph* cyclePatterns = NULL;
	struct Vertex* cyclePatternSearchTree = NULL;

	for (idx=biconnectedComponents; idx; idx=idx->next) {
		simpleCycles = addComponent(simpleCycles, listCycles(idx, sgp));
	}

	/* if cycles were found, compute canonical strings */
	if (simpleCycles) {
		/* transform cycle of shallow graphs to a list */
		simpleCycles->prev->next = NULL;
		simpleCycles->prev = NULL;

		cyclePatterns = getCyclePatterns(simpleCycles, sgp);
		cyclePatternSearchTree = buildSearchTree(cyclePatterns, gp, sgp);

		numCycles = cyclePatternSearchTree->number;
	} else {
		numCycles = 0;
	}



	if (numTrees + numCycles > *resSize) {
		if (*results) {
			free(*results);
		}

		*results = getResultVector(numTrees + numCycles);
		*resSize = numTrees + numCycles;
	}


	

	/* add elements to global search trees to obtain mapping from strings to integers */
	mergeSearchTrees(globalTreeSet, treePatternSearchTree, 1, *results, &pos, globalTreeSet, 0, gp);
	if (cyclePatternSearchTree) {
		mergeSearchTrees(globalTreeSet, cyclePatternSearchTree, 2, *results, &pos, globalTreeSet, 0, gp);
	}

	/* sort the output elements by increasing id */
	qsort(*results, pos, sizeof(struct compInfo), &compInfoComparison);

	/* output options */
	switch (outputOptions) {
	int i;

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
	case 'c':
		/* returns the number of cycles found in the graph */
		if (cyclePatternSearchTree)
			printf("%i %i\n", g->number, cyclePatternSearchTree->number / 2);
		else
			printf("%i 0\n", g->number);
		break;
	case 't':
		/* returns the number of trees found in the graph */
		printf("%i %i\n", g->number, treePatternSearchTree->number);
		break;
	case 'v':
		/* returns the number of vertices in the graph */
		printf("%i %i\n", g->number, g->n);
		break;
	case 'e':
		/* returns the number of edges in the graph */
		printf("%i %i\n", g->number, g->m);
		break;
	}


	/* garbage collection */

	/* dump cycles, if any
	 * TODO may be moved upwards directly after finding simple cycles */
	if (cyclePatternSearchTree) {
			dumpSearchTree(gp, cyclePatternSearchTree);
	}

	/* dump biconnected components list */
	for (idx=biconnectedComponents; idx; idx=tmp) {
		tmp = idx->next;
		dumpGraph(gp, idx);
	}

	dumpSearchTree(gp, treePatternSearchTree);
	dumpGraph(gp, forest);

	return 0;

}

/**
 * This is the actual main function that computes a feature vector from the graph g
 *  does not dump g */
int CyclicPatternKernel_onlyCycles(struct Graph *g, struct ShallowGraphPool *sgp, struct GraphPool *gp, struct Vertex* globalTreeSet, struct Vertex* globalCycleSet, struct compInfo** results, int* resSize) {
	struct Graph* tmp;
	struct Graph* idx;
	int numCycles;
	int pos = 0;
	int i;

	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	/* TODO refactor */
	struct Graph* biconnectedComponents = forest->next;

	/* list tree patterns */
	// struct ShallowGraph* treePatterns = getTreePatterns(forest, sgp);

	/* create search tree structure */
	// struct Vertex* treePatternSearchTree = buildSearchTree(treePatterns, gp, sgp);
	// int numTrees = treePatternSearchTree->number;

	/* list all cycles */
	struct ShallowGraph* simpleCycles = NULL;
	struct ShallowGraph* cyclePatterns = NULL;
	struct Vertex* cyclePatternSearchTree = NULL;

	for (idx=biconnectedComponents; idx; idx=idx->next) {
		simpleCycles = addComponent(simpleCycles, listCycles(idx, sgp));
	}

	/* if cycles were found, compute canonical strings */
	if (simpleCycles) {
		/* transform cycle of shallow graphs to a list */
		simpleCycles->prev->next = NULL;
		simpleCycles->prev = NULL;

		cyclePatterns = getCyclePatterns(simpleCycles, sgp);
		cyclePatternSearchTree = buildSearchTree(cyclePatterns, gp, sgp);

		numCycles = cyclePatternSearchTree->number;
	} else {
		numCycles = 0;
	}



	if (numCycles > *resSize) {
		if (*results) {
			free(*results);
		}

		*results = getResultVector(numCycles);
		*resSize = numCycles;
	}


	

	/* add elements to global search trees to obtain mapping from strings to integers */
	// mergeSearchTrees(globalTreeSet, treePatternSearchTree, 1, *results, &pos, globalTreeSet, 0, gp);
	if (cyclePatternSearchTree) {
		mergeSearchTrees(globalTreeSet, cyclePatternSearchTree, 2, *results, &pos, globalTreeSet, 0, gp);
		
		/* sort the output elements by increasing id */
		qsort(*results, pos, sizeof(struct compInfo), &compInfoComparison);
		
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
	} else {
		/* print only label */
		printf("%i", g->activity);
		printf("\n");
	}

	/* garbage collection */

	/* dump cycles, if any
	 * TODO may be moved upwards directly after finding simple cycles */
	if (cyclePatternSearchTree) {
		dumpSearchTree(gp, cyclePatternSearchTree);
	}

	/* dump biconnected components list */
	for (idx=biconnectedComponents; idx; idx=tmp) {
		tmp = idx->next;
		dumpGraph(gp, idx);
	}

	// dumpSearchTree(gp, treePatternSearchTree);
	dumpGraph(gp, forest);

	return 0;
}


/**
 * This is the actual main function that computes a feature vector from the graph g
 *  does not dump g */
int CyclicPatternKernel_onlyTrees(struct Graph *g, struct ShallowGraphPool *sgp, struct GraphPool *gp,
		struct Vertex* globalTreeSet, struct Vertex* globalCycleSet, struct compInfo** results, int* resSize) {
	struct Graph* tmp;
	struct Graph* idx;
	int pos = 0;
	int i;

	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	/* TODO refactor */
	struct Graph* biconnectedComponents = forest->next;

	/* list tree patterns */
	struct ShallowGraph* treePatterns = getTreePatterns(forest, sgp);

	/* create search tree structure */
	struct Vertex* treePatternSearchTree = buildSearchTree(treePatterns, gp, sgp);
	int numTrees = treePatternSearchTree->number;

	if (numTrees > *resSize) {
		if (*results) {
			free(*results);
		}

		*results = getResultVector(numTrees);
		*resSize = numTrees;
	}

	/* add elements to global search trees to obtain mapping from strings to integers */
	mergeSearchTrees(globalTreeSet, treePatternSearchTree, 1, *results, &pos, globalTreeSet, 0, gp);

	/* sort the output elements by increasing id */
	qsort(*results, pos, sizeof(struct compInfo), &compInfoComparison);

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

	/* garbage collection */

	/* dump biconnected components list */
	for (idx=biconnectedComponents; idx; idx=tmp) {
		tmp = idx->next;
		dumpGraph(gp, idx);
	}

	dumpSearchTree(gp, treePatternSearchTree);
	dumpGraph(gp, forest);

	return 0;
}
