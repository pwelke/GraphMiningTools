#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../listComponents.h"


struct Graph* getBlockTreeT(struct Graph* g, struct ShallowGraphPool* sgp) {

	char* isRoot = malloc(g->n * sizeof(char));
	int* smallestComponent = malloc(g->n * sizeof(int));
	int* parentRoot = malloc(g->n * sizeof(int));
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);

	for (int v=0; v<g->n; ++v) {
		isRoot[v] = 0;
		smallestComponent[v] = g->n + 1;
		parentRoot[v] = g->n + 1;
	}

	/* nonroots occur in exactly one biconnected component, roots occur in at least two components.
	 * (see tarjans paper DEPTH-FIRST SEARCH AND LINEAR GRAPH ALGORITHMS (1972) We can hence just count
	 * the number of occurrences and get the roots */
	int currentComp = 0;
	int nRoots = 0;
	for (struct ShallowGraph* bic=biconnectedComponents; bic!=NULL; bic=bic->next, ++currentComp) {
		for (struct VertexList* e=bic->edges; e!=NULL; e=e->next) {
			struct Vertex* v = e->startPoint;
			struct Vertex* w = e->endPoint;

			if (smallestComponent[v->number] < currentComp) {
				isRoot[v->number] = 1;
				++nRoots;
			} else {
				smallestComponent[v->number] = currentComp;
			}

			if (smallestComponent[w->number] < currentComp) {
				isRoot[w->number] = 1;
				++nRoots;
			} else {
				smallestComponent[w->number] = currentComp;
			}
		}
	}

	/* now, to build a tree from that, we mark for each vertex the lowest lowPoint of any vertex in any component that contains v.
	 * that is the parent root */
	for (struct ShallowGraph* bic=biconnectedComponents; bic!=NULL; bic=bic->next) {
		int minLowPoint = g->n + 1;
		struct Vertex* rootOfComponent = NULL;
		for (struct VertexList* e=bic->edges; e!=NULL; e=e->next) {
			struct Vertex* v = e->startPoint;
			struct Vertex* w = e->endPoint;

			if (minLowPoint > v->lowPoint) {
				minLowPoint = v->lowPoint;
				rootOfComponent = v;
			}

			if (minLowPoint > w->lowPoint) {
				minLowPoint = w->lowPoint;
			}
		}
		// TODO here, we should store the blocks directly with the corresponding roots.
		int rootId = rootOfComponent->number;
		for (struct VertexList* e=bic->edges; e!=NULL; e=e->next) {
			struct Vertex* v = e->startPoint;
			struct Vertex* w = e->endPoint;

			parentRoot[v->number] = rootId;
			parentRoot[w->number] = rootId;
		}
	}

	// want a tree on roots.
	// edges should go from parent to child
	// each vertex should store its parent vertex (where, in g or in the tree?)
	// each vertex should store all its v-rooted components
	// each vertex should store spanning trees of the v-rooted components
	// each vertex should store the set of characteristics
	// and maybe some pruning info?...

	struct Graph* rootTree = createGraph(nRoots, gp);
//	int i = 0;
	for (int v=0, i=0; v<g->n; ++v) {
		if (isRoot[v]) {
			struct Vertex* rootTreeVertex = rootTree->vertices[i];
			struct Vertex* originalVertex = g->vertices[v];

			rootTreeVertex->lowPoint = parentRoot[v];
			addEdge(,
		}
	}

//	for (int v=0; v<g->n; ++v) {
//		fprintf(stdout, "%i ", g->vertices[v]->lowPoint);
//	}
//	fprintf(stdout, "\n");
//	for (int v=0; v<g->n; ++v) {
//		fprintf(stdout, "%i ", isRoot[v]);
//	}
//	fprintf(stdout, "\n");
//	for (int v=0; v<g->n; ++v) {
//		fprintf(stdout, "%i ", isLeaf(g->vertices[v]));
//	}
//	fprintf(stdout, "\n");
//	for (int v=0; v<g->n; ++v) {
//		fprintf(stdout, "%i ", smallestComponent[v]);
//	}
//	fprintf(stdout, "\n");
//	for (int v=0; v<g->n; ++v) {
//		fprintf(stdout, "%i ", parentRoot[v]);
//	}
//	fprintf(stdout, "\n");

	for (int v=0; v<g->n; ++v) {
		if (isRoot[v]) {
			fprintf(stdout, "(%i -> %i) ", v, parentRoot[v]);
		}
	}
	fprintf(stdout, "\n");
	fprintf(stdout, "\n");

	return NULL;

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

			//			struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
			//			dumpShallowGraphCycle(sgp, biconnectedComponents);
			//
			//			for (int v=0; v<g->n; ++v) {
			//				fprintf(stdout, "%i ", g->vertices[v]->lowPoint);
			//			}
			//			fprintf(stdout, "\n");

			getBlockTreeT(g, sgp);

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


