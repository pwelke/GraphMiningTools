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
#include "labeled2unlabeledMain.h"


/**
 * Print --help message
 */
int printHelp() {
#include "../o/help/labeled2unlabeledMainHelp.help"
	unsigned char* help = executables_labeled2unlabeledMainHelp_txt;
	int len = executables_labeled2unlabeledMainHelp_txt_len;
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

void removeLabelsFromGraph(struct Graph* g, char* newLabel) {
	for (int vi=0; vi<g->n; ++vi) {
		struct Vertex* v = g->vertices[vi];
		if (v->isStringMaster) {
			free(v->label);
			v->isStringMaster = 0;
		}
		v->label = newLabel;

		for (struct VertexList* e=v->neighborhood; e!=NULL; e=e->next) {
			if (e->isStringMaster) {
				free(e->label);
				e->isStringMaster = 0;
			}
			e->label = newLabel;
		}
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

	char* label = "1";

	/* iterate over all graphs in the database */
	while ((g = iterateFile())) {
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n != -1) {
			
			removeLabelsFromGraph(g, label);
			printGraphAidsFormat(g, out);

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
