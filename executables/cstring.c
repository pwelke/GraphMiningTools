#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "../graph.h"
#include "../outerplanar.h"
#include "../cs_Tree.h"
#include "../cs_Parsing.h"
#include "../loading.h"
#include "../graphPrinting.h"
#include "../lwm_initAndCollect.h"
#include "cstring.h"


/**
 * Print --help message
 */
int printHelp() {
#include "../o/help/cstringHelp.help"
	unsigned char* help = executables_cstringHelp_txt;
	int len = executables_cstringHelp_txt_len;
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

	typedef enum {safe, unsafe, inverse} ConversionMethod;

	/* object pools */
	struct ListPool *lp;
	struct VertexPool *vp;
	struct ShallowGraphPool *sgp;
	struct GraphPool *gp;

	/* pointer to the current graph which is returned by the input iterator */
	struct Graph* g = NULL;
	int i = 0;

	/* pointer to graph database in case of inverse conversion */
	struct Graph** db = NULL;
	int nGraphs = 0;

	/* input and output */
	FILE* out = stdout;
	FILE* input = stdin;

	/* user set variables to specify what needs to be done */
	ConversionMethod method = safe;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hui";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
		case 'u':
			method = unsafe;
			break;
		case 'i':
			method = inverse;
			break;
		}
	}

	/* init object pools */
	lp = createListPool(10000);
	vp = createVertexPool(10000);
	sgp = createShallowGraphPool(1000, lp);
	gp = createGraphPool(100, vp, lp);

	// init
	switch (method) {
	case safe:
	case unsafe:

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
		break;

	case inverse:
		if (optind < argc) {
			char* filename = argv[optind];
			/* if the present filename is not '-' then init a file iterator for that file name */
			if (strcmp(filename, "-") != 0) {
				input = fopen(filename, "r");
			} else {
				input = stdin;
			}
		} else {
			input = stdin;
		}

		/** TODO if this breaks due to large input files, change this to an iterator */
		nGraphs = getDBfromCanonicalStrings(&db, input, CS_STRING_CACHE_SIZE, gp, sgp);
		fclose(input);
		break;
	}

	// processing
	switch (method) {

	case safe:
		/* iterate over all graphs in the database */
		while ((g = iterateFile())) {
			/* if there was an error reading some graph the returned n will be -1 */
			if (g->n != -1) {
				if (isTree(g)) {
					struct ShallowGraph* cString = canonicalStringOfTree(g, sgp);
					fprintf(out, "1\t%i\t", g->number);
					printCanonicalString(cString, out);
					dumpShallowGraph(sgp, cString);
				} else {
					fputs("No Tree\n", out);
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
		break;

	case unsafe:

		/* iterate over all graphs in the database */
		while ((g = iterateFile())) {
			/* if there was an error reading some graph the returned n will be -1 */
			if (g->n > 0) {
				struct ShallowGraph* cString = canonicalStringOfTree(g, sgp);
				fprintf(out, "1\t%i\t", g->number);
				printCanonicalString(cString, out);
				dumpShallowGraph(sgp, cString);

				/***** do not alter ****/

				++i;
				/* garbage collection */
				dumpGraph(gp, g);

			} else {
				/* TODO should be handled by dumpgraph */
				free(g);
			}
		}
		break;

	case inverse:

		for (i=0; i<nGraphs; ++i) {
			printGraphAidsFormat(db[i], out);
		}
		fprintf(out, "$\n");
		break;
	}

	/* global garbage collection */
	switch (method) {
	case safe:
	case unsafe:
		destroyFileIterator();
		break;
	case inverse:
		for (i=0; i<nGraphs; ++i) {
			dumpGraph(gp, db[i]);
		}
		free(db);
		break;
	}

	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}	
