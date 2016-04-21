#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../iterativeSubtreeIsomorphism.h"
#include "../subtreeIsoUtils.h"
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
				int* postorder = getPostorder(g, 0);
				for (int i=0; i<g->n; ++i) {
					printf("%i ", postorder[i] + 1);
				}
				printf("\n");
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
