#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../graphPrinting.h"
#include "neighborhoodGenerator.h"


/**
 * Print --help message
 */
static int printHelp() {
#include "neighborhoodGeneratorHelp.help"
	unsigned char* help = executables_neighborhoodGeneratorHelp_txt;
	int len = executables_neighborhoodGeneratorHelp_txt_len;
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


static char isInducedEdge(struct Graph* newGraph, struct VertexList* e) {
	int visitId = e->endPoint->visited;
	// sanity check: we did only set ->visited for neighbors of v, and don't know anything about other ->visiteds.
	char result = (visitId >=0) && (visitId < newGraph->n);
	// if the ->visited of the new copy equals the number of the endpoint, it must be a neighbor of v or v.
	return result && (newGraph->vertices[visitId]->visited == e->endPoint->number);
}


static void addInducedEdge(struct Graph* newGraph, struct VertexList* e, struct ListPool* lp) {
	struct VertexList* copy = getVertexList(lp);

	copy->label = e->label;
	copy->startPoint = newGraph->vertices[e->startPoint->visited];
	copy->endPoint = newGraph->vertices[e->endPoint->visited];
	addEdge(newGraph->vertices[e->startPoint->visited], copy);
	++newGraph->m;
}

/* uses ->visited of neighborhood of v */
struct Graph* getDiskGraph(struct Vertex* v, struct GraphPool* gp) {
	int i;
	int delta = degree(v);
	struct Graph* newGraph = createGraph(delta + 1, gp);
	struct Vertex* newV = newGraph->vertices[0];
	struct VertexList* e;

	// set ->visited in old graph to index of copy in newGraph
	// and ->visited in new graph to number of original in old graph
	v->visited = 0;
	newV->visited = v->number;
	newV->label = v->label;
	for (i=1, e=v->neighborhood; e!=NULL; ++i, e=e->next) {
		e->endPoint->visited = i;
		newGraph->vertices[i]->visited = e->endPoint->number;
		newGraph->vertices[i]->label = e->endPoint->label;
	}

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		struct VertexList* f;

		// add copy of edge from v to current neighbor to new graph
		addInducedEdge(newGraph, e, gp->listPool);

		// for each outgoing edge of current neighbor, check if it goes to the neighborhood of v
		// or to v itself
		for (f=e->endPoint->neighborhood; f!=NULL; f=f->next) {
			if (isInducedEdge(newGraph, f)) {
				addInducedEdge(newGraph, f, gp->listPool);
			}
		}
	}

	// each edge was added twice
	newGraph->m /= 2;

	// clean up visiteds of new graph
	for (i=0; i<delta+1; ++i) {
		newGraph->vertices[i]->visited = 0;
	}

	return newGraph;
}


/* uses ->visited of neighborhood of v */
struct Graph* getNeighborhoodGraph(struct Vertex* v, struct GraphPool* gp) {
	int i;
	int delta = degree(v);
	struct Graph* newGraph = createGraph(delta, gp);
	struct VertexList* e;

	// v is not contained in new graph
	v->visited = delta + 1;
	// set ->visited in old graph to index of copy in newGraph
	// and ->visited in new graph to number of original in old graph
	for (i=0, e=v->neighborhood; e!=NULL; ++i, e=e->next) {
		e->endPoint->visited = i;
		newGraph->vertices[i]->visited = e->endPoint->number;
		newGraph->vertices[i]->label = e->endPoint->label;
	}

	for (e=v->neighborhood; e!=NULL; e=e->next) {
		struct VertexList* f;

		// for each outgoing edge of current neighbor, check if it goes to the neighborhood of v
		// or to v itself
		for (f=e->endPoint->neighborhood; f!=NULL; f=f->next) {
			if (isInducedEdge(newGraph, f)) {
				addInducedEdge(newGraph, f, gp->listPool);
			}
		}
	}

	// each edge was added twice
	newGraph->m /= 2;

	// clean up visiteds of new graph
	for (i=0; i<delta; ++i) {
		newGraph->vertices[i]->visited = 0;
	}

	return newGraph;
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

	/* global variables */
	int i = 0;
	int neighborhoodCount = 1;
	struct Graph* g = NULL;

	/* output */
	FILE* out = stdout;

	/* user set variables to specify what needs to be done */
	int depth = 1;
	SubgraphSelector selector = disk;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hs:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
//		case 'd':
//			if (sscanf(optarg, "%i", &depth) != 1) {
//				fprintf(stderr, "value must be integer, is: %s\n", optarg);
//				return EXIT_FAILURE;
//			}
//			break;
		case 's':
			if (strcmp(optarg, "disk") == 0) {
				selector = disk;
				break;
			}
			if (strcmp(optarg, "neighbors") == 0) {
				selector = neighbors;
				break;
			}
			fprintf(stderr, "unknown argument: %s\n", optarg);
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
	gp = createGraphPool(1, vp, lp);

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

	struct Graph* (*subgraphSelector)(struct Vertex*, struct GraphPool*) = NULL;
	switch (selector) {
	case disk:
		subgraphSelector = &getDiskGraph;
		break;
	case neighbors:
		subgraphSelector = &getNeighborhoodGraph;
		break;
	}


	/* iterate over all graphs in the database */
	while ((g = iterateFile())) {
	
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n != -1) {
			int v;

			for (v=0; v<g->n; ++v, ++neighborhoodCount) {
				struct Graph* subgraph = NULL;

				subgraph = subgraphSelector(g->vertices[v], gp);

				subgraph->number = neighborhoodCount;
				subgraph->activity = atoi(g->vertices[v]->label);
				printGraphAidsFormat(subgraph, stdout);
				dumpGraph(gp, subgraph);
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

	/* terminate output stream */
	fprintf(out, "$\n");


	/* global garbage collection */
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}	
