#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../graphPrinting.h"


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


/* uses ->visited of neighborhood of v */
struct Graph* getNeighborhoodGraph(struct Vertex* v, struct GraphPool* gp) {
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
		struct VertexList* copy = getVertexList(gp->listPool);

		struct Vertex* currentNeighbor = e->endPoint;
		struct Vertex* currentNewNeighbor = newGraph->vertices[currentNeighbor->visited];
		
		// add copy of edge from v to current neighbor to new graph
		copy->label = e->label;
		copy->startPoint = newV;
		copy->endPoint = currentNewNeighbor;
		addEdge(newV, copy);
		++newGraph->m;

		// for each outgoing edge of current neighbor, check if it goes to the neighborhood of v
		// or to v itself
		for (f=currentNeighbor->neighborhood; f!=NULL; f=f->next) {
			int visitId = f->endPoint->visited;
			// sanity check: we did only set ->visited for neighbors of v, and don't know anything about other ->visiteds.
			if ((visitId >=0) && (visitId < delta+1)) {
				// if the ->visited of the new copy equals the number of the endpoint, it must be a neighbor of v or v.
				if (newGraph->vertices[visitId]->visited == f->endPoint->number) {
					struct Vertex* someNewNeighbor = newGraph->vertices[visitId];
					
					// add a copy for each such edge 
					copy = getVertexList(gp->listPool);
					copy->label = f->label;
					copy->startPoint = currentNewNeighbor;
					copy->endPoint = someNewNeighbor;
					addEdge(currentNewNeighbor, copy);
					++newGraph->m;
				}
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


	/* parse command line arguments */
	int arg;
	const char* validArgs = "hp:s:a:b:c:d:N:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
		case 'd':
			if (sscanf(optarg, "%i", &depth) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
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


	/* iterate over all graphs in the database */
	while ((g = iterateFile())) {
	
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n > 0) {
			int v;

			for (v=0; v<g->n; ++v, ++neighborhoodCount) {
				struct Graph* neighborhood = getNeighborhoodGraph(g->vertices[v], gp);
				neighborhood->number = neighborhoodCount;
				neighborhood->activity = atoi(g->vertices[v]->label);
				printGraphAidsFormat(neighborhood, stdout);
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
