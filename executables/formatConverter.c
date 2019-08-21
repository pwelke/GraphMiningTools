
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "../graph.h"
#include "../loading.h"

/**
 * Print --help message
 */
int printHelp() {
#include "../o/help/formatConverterHelp.help"
	unsigned char* help = executables_formatConverterHelp_txt;
	int len = executables_formatConverterHelp_txt_len;
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
 * Convert graph into gaston format and output it to out.
 * This method is called slow as there would be a way to convert the format much faster by not creating
 * an intermediate graph.
 */
void gastonConverterSlow(struct Graph* g, FILE* out) {
	fprintf(out, "t # %i", g->number);
	for (int v=0; v<g->n; ++v) {
		fprintf(out, "\nv %i ", v);
		fputs(g->vertices[v]->label, out);
	}
	for (int v=0; v<g->n; ++v) {
		for (struct VertexList* e=g->vertices[v]->neighborhood; e!=NULL; e=e->next) {
			if (e->startPoint->number < e->endPoint->number) {
				fprintf(out, "\ne %i %i ", e->startPoint->number, e->endPoint->number);
				fputs(e->label, out);
			}
		}
	}
	fputc('\n', out);
}


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
		if (g->n != -1) {

			gastonConverterSlow(g, stdout);
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
