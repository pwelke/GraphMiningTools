#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "../levelwiseGraphMining.h"

const char DEBUG_INFO = 1;


/**
 * Print --help message
 */
int printHelp() {
	FILE* helpFile = fopen("executables/levelwiseGraphMiningHelp.txt", "r");
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

	/* user input handling variables */
	int threshold = 1000;
	unsigned int maxPatternSize = 20;
	void (*miningStrategy)(size_t, size_t, FILE*, FILE*, FILE*, struct GraphPool*, struct ShallowGraphPool*) = &iterativeBFSMain;
	char* patternFile = NULL;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "ht:p:m:o:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
			break;
		case 't':
			if (sscanf(optarg, "%i", &threshold) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'p':
			if (sscanf(optarg, "%u", &maxPatternSize) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'm':
			if (strcmp(optarg, "dfs") == 0) {
				miningStrategy = &iterativeDFSMain;
				break;
			}
			if (strcmp(optarg, "bfs") == 0) {
				miningStrategy = &iterativeBFSMain;
				break;
			}
			fprintf(stderr, "Unknown mining technique: %s\n", optarg);
			return EXIT_FAILURE;
		case 'o':
			patternFile = optarg;
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

	// start frequent subgraph mining

	FILE* featureStream = stdout;
	FILE* patternStream = stdout;
	FILE* logStream = stderr;

	if (patternFile != NULL) {
		patternStream = fopen(patternFile, "w");
		fprintf(logStream, "Write patterns to file: %s\n", patternFile);
	}

	miningStrategy(maxPatternSize, threshold, featureStream, patternStream, logStream, gp, sgp);

	destroyFileIterator(); // graphs are in memory now

	if (patternFile != NULL) {
		fclose(patternStream);
	}

	/* global garbage collection */
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}
