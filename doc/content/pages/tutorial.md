Title: Tutorial
Date: 2015-12-01
Modified: 2015-12-01
Category: Documentation
Tags: pelican, publishing
Slug: Tutorial
Authors: Pascal Welke
Summary: This page shows how to write and build an executable using the smallgraph library.

On this page, we will create a simple application that parses a graph database in [our format]({file}/pages/fileformat.md) and checks if each graph is a tree.
We will 

- look at a template of an executable and explain its parts
- add the test if a graph is a tree
- write some documentation and 
- add the build recipe for the program to the Makefile


# A Template for an Executable that Loops Through the Graph Database

Below you can see the content of a source file that creates, when compiled, an executable that accepts a graph database
The smallgraph executables try to follow the unix way of doing things, where possible.
That means, command line arguments and input handling should behave as similar as possible to the usual stuff in the gnu coreutils package.
Hence, the graph database can be piped to the executable via stdin or via a file given as the last parameter of the executable.

To follow the structure of the smallgraphs project, the source of executables (those modules containing main() functions) should be saved in the /executables/ folder.

	#!C
	#include <stdio.h>
	#include <getopt.h>

	#include "../graph.h"
	#include "../loading.h"

	int main(int argc, char** argv) {

		/* object pools */
		struct ListPool *lp;
		struct VertexPool *vp;
		struct ShallowGraphPool *sgp;
		struct GraphPool *gp;

		/* pointer to the current graph which is returned by the input iterator  */
		struct Graph* g = NULL;
		/* stores number of graphs that were read */
		int i = 0;

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

Parsing of command line arguments (Line 20-32) is done by the functionality provided in getopt.h, which is provided by the GNU version of libc.
At the moment, the above code accepts a single command line argument, namely -h which should, by convention, print some help information about the program. 
We will implement this functionality below.

The smallgraph library has its own memory management system provided by memoryManagement.h.
Graphs, vertices, edges and "ShallowGraphs" (lists of edges) are held in object pools to avoid a huge amount of malloc and frees, which provide methods to get zero initialised objects and to dump them afterwards.
These pools need to be initialized before any graph is parsed or created by the program (Line 34-38) and properly destructed at the end of our program (Line 69-72).

Reading the graph database file (see [format description](/pages/fileformat.md)) is handled by loading.h.
Depending on the command line arguments, it either reads from stdin or from a file (Line 40-53). 
It provides an iterator that returns one graph at a time (Line 56).
loading.h supports graph databases of arbitrary size: 
Only the current graph is represented in main memory.
For most applications, like filtering graphs according to some property or computing a cyclic pattern kernel, it is only necessary to work on the current graph, hence the iterator approach is more flexible.


# Adding the Test if Graphs are Trees

The template above so far does nothing useful.
It just parses each graph in a given database once and dumps it afterwards.
We now want to add some functionality.

Lets check if the graphs in the database are trees (a connected graph that contains no cycles).
outerplanar.h contains a method called isTree(struct Graph* g) that does exactly this.
To use it, we need to include outerplanar.h

	#!C
	#include "../outerplanar.h"

and add something sensible instead of Line 60 to output the result, like:

	#!C
	if (isTree(g)) {
		printf("Graph %i is a tree\n", g->number);
	} else {
		printf("Graph %i is no tree\n", g->number);
	}

And voila, we have a program that tests which graph is a tree, and which is not a tree.
This, among many other tests, is already implemented in the gf executable whose source can be found in executables/filter.c.


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

