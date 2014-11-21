#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>

#include "../graph.h"
#include "../searchTree.h"
#include "../loading.h"
#include "../listSpanningTrees.h"
#include "../listComponents.h"
#include "../outerplanar.h"
#include "../upperBoundsForSpanningTrees.h"
#include "../subtreeIsomorphism.h"
#include "../graphPrinting.h"
#include "../treeCenter.h"
#include "../connectedComponents.h"
#include "../cs_Tree.h"
#include "../wilsonsAlgorithm.h"
#include "filter.h"


/**
 * Print --help message
 */
void printHelp() {
printf("
A stupid graph database processor that can evaluate some graph properties.\n
Written 2014 by Pascal Welke in an attempt to clean up the mess of 3 years\n
of writing theses.\n
\n
Usage: gf [options] [FILE]\n
\n
By default, or if FILE is '-', gf reads from stdin. It always outputs to\n
stdout.\n
\n
For each graph in the input database, a measure, specified by the -f flag\n
is computed and compared to a value, specified by the -v flag by means of\n
a comparator specified by the -c flag. The output of gf is either the db \n
of all graphs that satisfy the above condition or a list of those values \n
or graph ids, which can be specified by the -o flag.\n
\n
options: options are always followed by a value.\n
\t-v 'value': an integer (default -1)\n
\t-c 'comparator': (default <=)\n
\t\t<=\n
\t\t==\n
\t\t>=\n
\t\t!=\n
\t\t<\n
\t\t>\n
\t-f 'filter': specify which property of the graph or db is to be used\n
\t             for comparison. (default count)\n
\t\t*counting*\n
\t\tgraphName\tthe graph ids, or names, specified in the database\n
\t\tcount\trunning number of the graph (e.g. select the first 10 graphs)\n
\t\t\t \n
\t\t*labels* (labels are integer in our db format)\n
\t\tlabel\tthe label of a graph, specified in the graph db\n
\t\tAvsI\tfor AIDS99-target attributes\n
\t\tAMvsI\tfor AIDS99-target attributes\n
\t\tAvsMI\tfor AIDS99-target attributes\n
\t\t\t \n
\t\t*boolean properties* (return 0 or 1)\n
\t\tconnected\tcheck if a graph is connected\n
\t\touterplanar\t check if a graph is outerplanar\n
\t\ttree\t check if a graph is a tree\n
\t\tcactus\t check if a graph is a connected cactus graph\n
\t\t\t \n
\t\t*numerical properties*\n
\t\tspanningTreeEstimate\tcompute an upper bound on the number of\n
\t\t\t\tspanning trees in a graph\n
\t\tnumberOfBlocks\tcompute the number of biconnected blocks\n
\t\tnumberOfBridges\tcompute the number of bridges in a graph\n
\t\t\t \n
\t\t*TODO additional Parameter needed*\n
\t\tspanningTreeListing\t \n
\n
");
}


/**
 * Input handling, parsing of database and call of opk feature extraction method.
 */
int main(int argc, char** argv) {

	/* create object pools */
	struct ListPool *lp = createListPool(10000);
	struct VertexPool *vp = createVertexPool(10000);
	struct ShallowGraphPool *sgp = createShallowGraphPool(1000, lp);
	struct GraphPool *gp = createGraphPool(100, vp, lp);

	/* pointer to the current graph which is returned by the input iterator */
	struct Graph* g = NULL;
	int i = 0;

	/* output */
	FILE* out = stdout;

	/* user set variables to specify what needs to be done */
	Filter filter = count;
	Comparator comparator = geq;
	OutputOption oOption = graph;
	int value = -1;

	/* parse command line arguments */
	int arg;
	const char* validArgs = "hf:c:v:o:";
	for (arg=getopt(argc, argv, validArgs); arg!=-1; arg=getopt(argc, argv, validArgs)) {
		switch (arg) {
		case 'h':
			printHelp();
			return EXIT_SUCCESS;
		case 'f':
			/* counting */
			if (strcmp(optarg, "graphName") == 0) {
				filter = graphName;
				break;
			}
			if (strcmp(optarg, "count") == 0) {
				filter = count;
				break;
			}
			if (strcmp(optarg, "connected") == 0) {
				filter = connected;
				break;
			}
			if (strcmp(optarg, "outerplanar") == 0) {
				filter = outerplanar;
				break;
			}
			if (strcmp(optarg, "tree") == 0) {
				filter = tree;
				break;
			}
			if (strcmp(optarg, "cactus") == 0) {
				filter = cactus;
				break;
			}
			if (strcmp(optarg, "spanningTreeEstimate") == 0) {
				filter = spanningTreeEstimate;
				break;
			}
			if (strcmp(optarg, "numberOfBlocks") == 0) {
				filter = numberOfBlocks;
				break;
			}
			if (strcmp(optarg, "numberOfBridges") == 0) {
				filter = outerplanar;
				break;
			}
			fprintf(stderr, "Unknown filter: %s\n", optarg);
			return EXIT_FAILURE;
			break;
		case 'c':
			if (strcmp(optarg, "==") == 0) {
				comparator = eq;
				break;
			}
			if (strcmp(optarg, "<=") == 0) {
				comparator = leq;
				break;
			}
			if (strcmp(optarg, ">=") == 0) {
				comparator = geq;
				break;
			}
			if (strcmp(optarg, "<") == 0) {
				comparator = less;
				break;
			}
			if (strcmp(optarg, ">") == 0) {
				comparator = greater;
				break;
			}
			fprintf(stderr, "Unknown comparator: %s\n", optarg);
			return EXIT_FAILURE;
			break;
		case 'v':
			if (sscanf(optarg, "%i", &value) != 1) {
				fprintf(stderr, "value must be integer, is: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'o':
			if ((strcmp(optarg, "graph") == 0) || (strcmp(optarg, "g") == 0)) {
				oOption = graph;
				break;
			}
			if ((strcmp(optarg, "idAndValue") == 0) || (strcmp(optarg, "iv") == 0)) {
				oOption = idAndValue;
				break;
			}
			if ((strcmp(optarg, "id") == 0) || (strcmp(optarg, "i") == 0)) {
				oOption = id;
				break;
			}
			if ((strcmp(optarg, "value") == 0) || (strcmp(optarg, "v") == 0)) {
				oOption = value;
				break;
			}
			fprintf(stderr, "Unknown output option: %s\n", optarg);
			return EXIT_FAILURE;
			break;
		case '?':
			return EXIT_FAILURE;
			break;
		}
	}

	/* initialize the stream to read graphs from */
	createStdinIterator(gp);
	char* filename = "stdin";
	/* check if there is a filename present in the command line arguments */
	if (optind < argc) {
		filename = argv[optind];
		/* if the present filename is not '-' then init a file iterator for that file name */
		if (strcmp(filename, "-") != 0) {
			createFileIterator(filename, gp);
		} 
	}

	// // debug
	// fprintf(stdout, "filter=%i, comparator=%i, oOption=%i, value=%i, stream=%s\n", filter, comparator, oOption, value, filename);

	/* iterate over all graphs in the database */
	while ((g = iterateFile(&aids99VertexLabel, &aids99EdgeLabel))) {
		/* if there was an error reading some graph the returned n will be -1 */
		if (g->n > 0) {
			
			processGraph(i, g, filter, comparator, value, out, oOption, sgp, gp);

			/***** do not alter ****/

			++i;
			/* garbage collection */
			dumpGraph(gp, g);

		} else {
			/* TODO should be handled by dumpgraph */
			free(g);
		}
	}

	/* terminate output stream, if we write graphs */
	if (oOption == graph) {
		fprintf(out, "\n$\n");
	}

	/* global garbage collection */
	destroyFileIterator();
	freeGraphPool(gp);
	freeShallowGraphPool(sgp);
	freeListPool(lp);
	freeVertexPool(vp);

	return EXIT_SUCCESS;
}	

void processGraph(int i, struct Graph* g, Filter filter, Comparator comparator, int value, FILE* out, OutputOption oOption, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	int measure = -1;
	switch (filter) {

	/* counts */ 
	case count:
		// TODO break condition for <= ==
		if (conditionHolds(i, value, comparator)) {
			output(g, i, oOption, out);
		}
		break;
	case graphName:
		if (conditionHolds(g->number, value, comparator)) {
			output(g, g->number, oOption, out);
		}
		break;

	/* labels */
	case label:
		if (conditionHolds(g->activity, value, comparator)) {
			output(g, g->activity, oOption, out);
		}
		break;
	case AvsI:
		if (g->activity == 1) {
			break;
		}
		if (g->activity == 0) {
			g->activity = -1;
		}
		if (g->activity == 2) {
			g->activity = 1;
		}
		if (conditionHolds(g->activity, value, comparator)) {
			output(g, g->activity, oOption, out);
		}
		break;
	case AvsMI:
		if (g->activity == 2) {
			g->activity = 1;
		} else {
			g->activity = -1;
		}
		if (conditionHolds(g->activity, value, comparator)) {
			output(g, g->activity, oOption, out);
		}
		break;
	case AMvsI:
		if (g->activity == 0) {
			g->activity = -1;
		}
		if (g->activity == 2) {
			g->activity = 1;
		}
		if (conditionHolds(g->activity, value, comparator)) {
			output(g, g->activity, oOption, out);
		}
		break;

	/* boolean properties */
	case connected:
		measure = isConnected(g);
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	case outerplanar:
		// isMaximalOuterplanar alters g. do not attempt to do something with g after this.
		measure = isOuterplanarGraph(g, sgp, gp);
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	case tree:
		measure = isTree(g);
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	case cactus:
		measure = isCactus(g, sgp);
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;

	/* numeric properties */
	case spanningTreeEstimate:
		measure = getGoodEstimate(g, sgp, gp);
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	case numberOfBlocks:
		measure = getNumberOfBlocks(g, sgp);			
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	case numberOfBridges:
		measure = getNumberOfBridges(g, sgp);			
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	}
}

/**
Check if a condition of the form measure comparator threshold holds.
E.g. measure <= threshold.
*/
char conditionHolds(int measure, int threshold, Comparator comparator) {
	switch (comparator) {
	case leq:
		return measure <= threshold;
		break;
	case eq:
		return measure == threshold;
		break;
	case geq:
		return measure >= threshold;
		break;
	case neq:
		return measure != threshold;
		break;
	case less:
		return measure < threshold;
		break;
	case greater:
		return measure > threshold;
		break;
	}
	// should not happen
	return 0;
}

void output(struct Graph* g, int measure, OutputOption option, FILE* out) {
	switch (option) {
	case graph:
		writeCurrentGraph(out);
		break;
	case idAndValue:
		fprintf(out, "%i %i\n", g->number, measure);
		break;
	case value:
		fprintf(out, "%i\n", measure);
	case id:
		fprintf(out, "%i\n", g->number);
	}
} 

/**
Count the number of biconnected components that are not a bridge.
*/
int getNumberOfBlocks(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	int compNumber = 0;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			++compNumber;
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return compNumber;
}

/**
Count the number of edges in the graph that are bridges. 
I.e. count the number of biconnected components with only one edge.
*/
int getNumberOfBridges(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	int bridgeNumber = 0;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m == 1) {
			++bridgeNumber;
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return bridgeNumber;
}

/**
A tree is a connected graph with m = n-1 edges.
*/
char isTree(struct Graph* g) {
	if (isConnected(g)) {
		return g->m == g->n - 1;
	} else { 
		return 0;
	}
}

/**
A cactus is a connected graph where each nontrivial biconnected block (i.e., a
biconnected component that is not a bridge) is a simple cycle.
*/
char isCactus(struct Graph* g, struct ShallowGraphPool* sgp) {
	if (isConnected(g)) {
		struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
		struct ShallowGraph* comp;
		int compNumber = 0;
		for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
			if (comp->m > 1) {
				++compNumber;
			}			
		}
		/* cleanup */
		dumpShallowGraphCycle(sgp, biconnectedComponents);
		return compNumber;
	} else {
		return 0;
	}
}

/**
An outerplanar graph is a graph that can be drawn in the plane such that
(1) edges only intersect at vertices and
(2) each vertex can be reached from the outer face without crossing an edge.

A graph is outerplanar if and only if each of its biconnected components is outerplanar.

TODO

outerplanar.[ch]

isMaximalOuterplanar() -> isOuterplanarBlock()
isOuterplanar() -> isOuterplanarBlockShallow()
isOuterplanarGraph() -> isOuterplanar()

*/ 
char isOuterplanarGraph(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);
	struct ShallowGraph* comp;
	char isOp = 1;
	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			isOp = isOuterplanar(comp, sgp, gp);
			if (isOp == 0) {
				break;
			}
		}			
	}
	/* cleanup */
	dumpShallowGraphCycle(sgp, biconnectedComponents);
	return isOp;
}
