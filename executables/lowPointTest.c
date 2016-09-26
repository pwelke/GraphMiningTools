#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../listComponents.h"
#include "../wilsonsAlgorithm.h"
#include "../graphPrinting.h"


/*
 want a tree on roots.
 edges should go from parent to child
 [x] each vertex should store its parent vertex (where, in g or in the tree?)
 [x] each vertex should store all its v-rooted components
 each vertex should store spanning trees of the v-rooted components
 each vertex should store the set of characteristics
 and maybe some pruning info?... */
struct BlockTree{
	struct Graph* g;
	struct Vertex** roots;
	struct Vertex** parents;
	struct ShallowGraph** vRootedBlocks;
	int nRoots;
};

struct SpanningtreeTree{
	struct Graph* g;
	struct Vertex** roots;
	struct Vertex** parents;
	struct Graph** blocks;
	struct ShallowGraph** blockSpanningTrees;
	int nRoots;
};

/* return the head of the list or NULL if list is empty.
 * remove head of list
 * (for speeds sake, don't change pointers
 */
static struct ShallowGraph* popShallowGraph(struct ShallowGraph** list) {
	struct ShallowGraph* head = *list;
	if (head != NULL) {
		*list = (*list)->next;
	}
	return head;
}

static int lowPointComparator(const void* a, const void* b) {
	struct Vertex* v = *(struct Vertex**)a;
	struct Vertex* w = *(struct Vertex**)b;

	return v->lowPoint - w->lowPoint;
}

struct BlockTree getBlockTreeT(struct Graph* g, struct ShallowGraphPool* sgp) {

	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);

	for (int v=0; v<g->n; ++v) {
		g->vertices[v]->visited = 0; // isRoot
		g->vertices[v]->d = -1; // parent root
	}

	/* we mark for each component the vertex with the lowest lowPoint.
	 * that is the root of the component.
	 *
	 * Due to the order in which listBiconnectedComponents() returns the bics,
	 * we obtain the right parent roots for the roots in this way. */

	for (struct ShallowGraph* bic=biconnectedComponents; bic!=NULL; bic=bic->next) {

		// listBiconnectedComponents returns bics where the first vertex in the first edge is the root of each bic
		struct Vertex* rootOfComponent = bic->edges->startPoint;

		// mark the root of this component as root.
		rootOfComponent->visited = 1;

		// store the root as information with each vertex and the component
		int rootId = rootOfComponent->number;
		bic->data = rootId;
		for (struct VertexList* e=bic->edges; e!=NULL; e=e->next) {
			e->startPoint->d = rootId;
			e->endPoint->d = rootId;
		}
	}

	// create output struct
	struct BlockTree blockTree = {0};
	blockTree.g = g;

	// count number of roots in g, init storage
	for (int v=0; v<g->n; ++v) {
		if (g->vertices[v]->visited) {
			++blockTree.nRoots;
		}
	}
	blockTree.roots = malloc(blockTree.nRoots * sizeof(struct Vertex*));
	blockTree.parents = malloc(blockTree.nRoots * sizeof(struct Vertex*));
	blockTree.vRootedBlocks = malloc(blockTree.nRoots * sizeof(struct ShallowGraph*));

	// select and sort roots by lowpoint. this ensures bottom up traversal of the
	// blocktree if iterating through the array
	for (int v=0, r=0; v<g->n; ++v) {
		if (g->vertices[v]->visited) {
			blockTree.roots[r] = g->vertices[v];
			++r;
		}
	}
	qsort(blockTree.roots, blockTree.nRoots, sizeof(struct Vertex*), &lowPointComparator);

	// add parents of roots to array (after sorting the above, of course)
	// init vRootedBlocks to NULL
	// set root->visited to its position in roots (so that we can map the bics efficiently)
	for (int v=0; v<blockTree.nRoots; ++v) {
		blockTree.parents[v] = g->vertices[blockTree.roots[v]->d];
		blockTree.vRootedBlocks[v] = NULL;
		blockTree.roots[v]->visited = v;
	}

	// add blocks to respective roots
	for (struct ShallowGraph* bic=popShallowGraph(&biconnectedComponents); bic!=NULL; bic=popShallowGraph(&biconnectedComponents)) {
		int rootPositionInBlockTreeArrays = g->vertices[bic->data]->visited;
		bic->next = blockTree.vRootedBlocks[rootPositionInBlockTreeArrays];
		blockTree.vRootedBlocks[rootPositionInBlockTreeArrays] = bic;
	}


//	for (int v=0; v<blockTree.nRoots; ++v) {
//		fprintf(stdout, "%i ", blockTree.roots[v]->lowPoint);
//	}
//	fprintf(stdout, "\n");
//	for (int v=0; v<blockTree.nRoots; ++v) {
//		fprintf(stdout, "%i->%i:\n", blockTree.roots[v]->number, blockTree.parents[v]->number);
//		printShallowGraph(blockTree.vRootedBlocks[v]);
//	}
//	fprintf(stdout, "\n");
//	for (int v=0; v<blockTree.nRoots; ++v) {
//		printShallowGraph(blockTree)
//	}
//	fprintf(stdout, "\n");
//	fprintf(stdout, "\n");

	return blockTree;
}

/**
 * Merge two shallow graphs. The the result is the first shallow graph in list with
 * all the edges added, the second shallow graph is dumped.
 */
static struct ShallowGraph* mergeTwoShallowGraphs(struct ShallowGraph* first, struct ShallowGraph* second, struct ShallowGraphPool* sgp) {
	first->lastEdge->next = second->edges;
	first->lastEdge = second->lastEdge;
	first->m += second->m;

	second->edges = second->lastEdge = NULL;
	second->next = NULL;
	dumpShallowGraph(sgp, second);
	return first;
}


/**
 * Merge all shallow graphs in the list. The list is consumed, the result is the first shallow graph in list with
 * all the edges added.
 */
static struct ShallowGraph* mergeShallowGraphs(struct ShallowGraph* list, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* head = popShallowGraph(&list);
	head->next = NULL;

	for (struct ShallowGraph* pop=popShallowGraph(&list); pop!=NULL; pop=popShallowGraph(&list)) {
		mergeTwoShallowGraphs(head, pop, sgp);
	}
	return head;
}

struct SpanningtreeTree getSpanningtreeTree(struct BlockTree blockTree, int spanningTreesPerBlock, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	struct SpanningtreeTree sptTree = {0};
	sptTree.g = blockTree.g;
	sptTree.nRoots = blockTree.nRoots;
	sptTree.roots = blockTree.roots;
	sptTree.parents = blockTree.parents;
	sptTree.blocks = malloc(sptTree.nRoots * sizeof(struct Graph*));
	sptTree.blockSpanningTrees = malloc(sptTree.nRoots * sizeof(struct ShallowGraph*));

	for (int v=0; v<sptTree.nRoots; ++v) {
		struct ShallowGraph* mergedEdges = mergeShallowGraphs(blockTree.vRootedBlocks[v], sgp);
		struct Graph* mergedGraph = shallowGraphToGraph(mergedEdges, gp);
		sptTree.blocks[v] = mergedGraph;

//		printf("%i:\n", blockTree.roots[v]->number);
//		printShallowGraph(mergedEdges);

		if (mergedGraph->m != mergedGraph->n-1) {
			sptTree.blockSpanningTrees[v] = NULL;
			// sample spanning trees according to parameter
			for (int i=0; i<spanningTreesPerBlock; ++i) {
				struct ShallowGraph* spt = randomSpanningTreeAsShallowGraph(mergedGraph, sgp);
				spt->next = sptTree.blockSpanningTrees[v];
				sptTree.blockSpanningTrees[v] = spt;
			}
			dumpShallowGraph(sgp, mergedEdges);
		} else {
			// the block is a tree, we don't need to do anything
			sptTree.blockSpanningTrees[v] = mergedEdges;
		}
//		printf("spanning trees:\n");
//		printShallowGraph(sptTree.blockSpanningTrees[v]);
	}

	/* garbage collection */
	free(blockTree.vRootedBlocks);
	return sptTree;
}



int main(int argc, char** argv) {

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	/* pointer to the current graph which is returned by the input iterator  */
	struct Graph* g = NULL;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "h";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			//				printHelp();
			return EXIT_SUCCESS;
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

			struct BlockTree blockTree = getBlockTreeT(g, sgp);
			struct SpanningtreeTree sptTree = getSpanningtreeTree(blockTree, 2, gp, sgp);

			free(sptTree.roots);
			free(sptTree.parents);
			for (int v=0; v<sptTree.nRoots; ++v) {
				dumpShallowGraphCycle(sgp, sptTree.blockSpanningTrees[v]);
				dumpGraph(gp, sptTree.blocks[v]);
			}
			free(sptTree.blocks);
			free(sptTree.blockSpanningTrees);

		}
		/* garbage collection */
		dumpGraph(gp, g);
	}

	/* global garbage collection */
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}


