#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../iterativeSubtreeIsomorphism.h"
#include "../subtreeIsomorphism.h"
#include "../treeEnumeration.h"



	/**
	 * Print --help message
	 */
	int printHelp() { return 0; }

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
				printHelp();
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

				struct VertexList* edge = getVertexList(lp);
				edge->startPoint = getVertex(vp);
				edge->endPoint = getVertex(vp);
				edge->startPoint->label = intLabel(2);
				edge->label = intLabel(1);
				edge->endPoint->label = intLabel(1);

				edge->isStringMaster = 1;
				edge->startPoint->isStringMaster = 1;
				edge->endPoint->isStringMaster = 1;

				struct SubtreeIsoDataStore base = initG(g);
				struct SubtreeIsoDataStore one = initIterativeSubtreeCheck(base, edge, gp);

				printf("lenght 1, Found Iso: %i\n", one.foundIso);
				printNewCubeCondensed(one.S, one.g->n, one.h->n);

				struct SubtreeIsoDataStore prev = one;
				for (int i=1; i<2; ++i) {
					struct Graph* ext = refinementGraph(prev.h, 0, edge, gp);
					struct SubtreeIsoDataStore current = iterativeSubtreeCheck(prev, ext, gp);

					printf("lenght %i, Found Iso: %i\n", current.h->m, current.foundIso);
					printNewCubeCondensed(current.S, current.g->n, current.h->n);

					dumpNewCube(prev.S, prev.g->n, prev.h->n);
					dumpGraph(gp, prev.h);

					prev = current;
				}


				dumpNewCube(prev.S, prev.g->n, prev.h->n);
				dumpGraph(gp, prev.h);
				
				// printNewCube(next.S, next.g->n, next.h->n);

				// garbage collection
				// dumpNewCube(next.S, next.g->n, next.h->n);
				// dumpGraph(gp, next.h);				

				// dumpNewCube(one.S, one.g->n, one.h->n);
				// dumpGraph(gp, one.h);
				free(base.postorder);

				dumpVertex(vp, edge->startPoint);
				dumpVertex(vp, edge->endPoint);
				dumpVertexList(lp, edge);
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
