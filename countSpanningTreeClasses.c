#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "graph.h"
#include "searchTree.h"
#include "listSpanningTrees.h"
#include "upperBoundsForSpanningTrees.h"
#include "subtreeIsomorphism.h"
#include "levelwiseMain.h" 
#include "levelwiseMining.h"



/**
 * Print --help message
 */
void printHelp() {
	printf("This is a Levelwise Algorithm for TreePatterns\n");
	printf("implemented by Pascal Welke 2013\n\n\n");
	printf("usage: lwm F [parameterList]\n\n");
	printf("    without parameters: display this help screen\n\n");
	printf("    F: (required) use F as graph database\n\n");
	printf("    -output O: write output to stdout\n");

	printf("    -limit N: process the first N graphs in F\n\n");
	printf("    -h | --help: display this help\n\n");
}


/**
 Main method of the TreePatternKernel levelwise pattern generation algorithm.
 It will use a database of spanning trees generated by the preprocessing 
 algorithm accompanying it.
 */
int main(int argc, char** argv) {
	if ((argc < 2) || (strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0)) {
		printHelp();
		return EXIT_FAILURE;
	} else {

		/* create object pools */
		struct ListPool *lp = createListPool(1);
		struct VertexPool *vp = createVertexPool(1);
		struct ShallowGraphPool *sgp = createShallowGraphPool(1, lp);
		struct GraphPool *gp = createGraphPool(1, vp, lp);

		/* init params */
		char* inputFileName = argv[1];	
		FILE* stream = fopen(inputFileName, "r");
		int bufferSize = 20;
		int number;
		struct ShallowGraph* patterns;

		while ((patterns = streamReadPatterns(stream, bufferSize, &number, sgp))) {
			int n = 0;
			struct ShallowGraph* p;
			for (p=patterns; p!=NULL; p=p->next) {
				++n;
			}
			printf("%i %i\n", number, n);
			dumpShallowGraphCycle(sgp, patterns);
		}

		/* garbage collection */
		freeGraphPool(gp);
		freeShallowGraphPool(sgp);
		freeListPool(lp);
		freeVertexPool(vp);

		return EXIT_SUCCESS;
	}
}
