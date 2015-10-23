#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"
#include "../graphPrinting.h"
#include "../searchTree.h"
#include "../weisfeilerLehman.h"


/**
 * Print --help message
 */
int printHelp() {
#include "weisfeilerLehmanMainHelp.help"
	unsigned char* help = executables_weisfeilerLehmanMainHelp_txt;
	int len = executables_weisfeilerLehmanMainHelp_txt_len;
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
	int i = 0;
	struct Vertex* wlLabels;

	/* output */
	FILE* out = stdout;

	/* user set variables to specify what needs to be done */
	// Filter filter = count;
	// Comparator comparator = pass;
	// OutputOption oOption = graph;
	// int value = -1;
	// // can be set via -a. Used e.g. by spanningTreeListing filter, and randomSample
	// int additionalParameter = 100;

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

	/* init wl label compression search tree */
	wlLabels = getVertex(vp);

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
			
			struct Graph* h = weisfeilerLehmanRelabel(g, wlLabels, gp, sgp);
			printGraphAidsFormat(h, out);
			dumpGraph(gp, h);

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
	dumpSearchTree(gp, wlLabels);
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}	
