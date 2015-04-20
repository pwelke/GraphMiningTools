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
#include "cstring.h"


/**
 * Print --help message
 */
int printHelp() {
	FILE* helpFile = fopen("executables/cstringHelp.txt", "r");
	if (helpFile != NULL) {
		int c = EOF;
		while ((c = fgetc(helpFile)) != EOF) {
			fputc(c, stdout);
		}
		fclose(helpFile);
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Could not read helpfile\n");
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

	/* output */
	FILE* out = stdout;

	/* user set variables to specify what needs to be done */
	char*(*vertexLabelFunction)(const unsigned int) = &intLabel;
	char*(*edgeLabelFunction)(const unsigned int) = &intLabel;
	char safe = 1;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hlu";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
		case 'l':
			vertexLabelFunction = &aids99VertexLabel;
			edgeLabelFunction = &aids99EdgeLabel;
			break;
		case 'u':
			safe = 0;
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

	if (safe) {
		/* iterate over all graphs in the database */
		while ((g = iterateFile(vertexLabelFunction, edgeLabelFunction))) {
			/* if there was an error reading some graph the returned n will be -1 */
			if (g->n > 0) {
				if (isTree(g)) {
					struct ShallowGraph* cString = canonicalStringOfTree(g, sgp);
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
	} else {
		/* iterate over all graphs in the database */
		while ((g = iterateFile(vertexLabelFunction, edgeLabelFunction))) {
			/* if there was an error reading some graph the returned n will be -1 */
			if (g->n > 0) {
				struct ShallowGraph* cString = canonicalStringOfTree(g, sgp);
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
	}

	/* global garbage collection */
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}	
