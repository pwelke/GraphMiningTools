Title: Tutorial
Date: 2015-12-01
Modified: 2015-12-01
Category: Documentation
Tags: pelican, publishing
Slug: Tutorial
Authors: Pascal Welke
Summary: This page shows how to write and build an executable using the smallgraph library.

# A Simple Template

On this page, we will create a simple application that parses a graph database in [our format]({file}/pages/fileformat.md).

	#!C
	#include <stdio.h>
	#include <ctype.h>
	#include <stdlib.h>
	#include <string.h>
	#include <getopt.h>

	#include "graph.h"
	#include "loading.h"
	#include "graphPrinting.h"

	/**
	 * Input handling, parsing of database and processing.
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

				// DO YOUR STUFF HERE

				/***** do not alter ****/

				++i;
				/* garbage collection */
				dumpGraph(gp, g);

			} else {
				/* TODO should be handled by dumpgraph */
				free(g);
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



# Documenting the Interface of your Program
	
	#!C
	/**
	 * Print --help message
	 */
	int printHelp() {
		FILE* helpFile = fopen("mainHelp.txt", "r");
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


# Adding a Make Target for Your Program

