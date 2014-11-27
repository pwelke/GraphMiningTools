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
int printHelp() {
	FILE* helpFile = fopen("executables/filterHelp.txt", "r");
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
	Filter filter = count;
	Comparator comparator = pass;
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
				filter = numberOfBridges;
				break;
			}
			if (strcmp(optarg, "maxCycleDegree") == 0) {
				filter = maxCycleDegree;
				break;
			}
			if (strcmp(optarg, "minCycleDegree") == 0) {
				filter = minCycleDegree;
				break;
			}
			if (strcmp(optarg, "maxDegree") == 0) {
				filter = maxDegree;
				break;
			}
			if (strcmp(optarg, "minDegree") == 0) {
				filter = minDegree;
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
			if (strcmp(optarg, "!=") == 0) {
				comparator = neq;
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

	// debug
	// fprintf(stderr, "filter=%i, comparator=%i, oOption=%i, value=%i, stream=%s\n", filter, comparator, oOption, value, filename);

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
		fprintf(out, "$\n");
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
	case maxDegree:
		measure = getMaxDegree(g);
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	case minDegree:
		measure = getMinDegree(g);
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	case maxCycleDegree:
		measure = getMaxCycleDegree(g, sgp);
		if (conditionHolds(measure, value, comparator)) {
			output(g, measure, oOption, out);
		}
		break;
	case minCycleDegree:
		measure = getMinCycleDegree(g, sgp);
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
	case pass:
		return 1;
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
Compute the maximum degree of any vertex in g.
A graph without vertices has maxdegree -1.

This method can handle graphs that are not full, i.e. there
are positions in the g->vertices array that are NULL.
*/
int getMaxDegree(struct Graph* g) {
	int max = -1;
	int v;
	for (v=0; v<g->n; ++v) {
		if (g->vertices[v] != NULL) {
			int deg = degree(g->vertices[v]);
			if (max < deg) {
				max = deg;
			}
		}
	}
	return max;
}


/**
Compute the minimum degree of any vertex in g.
A graph without vertices has mindegree INT_MAX.

This method can handle graphs that are not full, i.e. there
are positions in the g->vertices array that are NULL.
*/
int getMinDegree(struct Graph* g) {
	int min = INT_MAX;
	int v;
	for (v=0; v<g->n; ++v) {
		if (g->vertices[v] != NULL) {
			int deg = degree(g->vertices[v]);
			if (min > deg) {
				min = deg;
			}
		}
	}
	return min;
}

int* computeCycleDegrees(struct Graph* g, struct ShallowGraphPool* sgp) {
	struct ShallowGraph* biconnectedComponents = listBiconnectedComponents(g, sgp);

	/* store for each vertex if the current bic.comp was already counted */
	int* occurrences = malloc(g->n * sizeof(int));
	/* store the cycle degrees of each vertex in g */
	int* cycleDegrees = malloc(g->n * sizeof(int));

	int v;
	struct ShallowGraph* comp;
	int compNumber = 0;

	for (v=0; v<g->n; ++v) {
		occurrences[v] = -1;
		cycleDegrees[v] = 0;
	}

	for (comp = biconnectedComponents; comp!=NULL; comp=comp->next) {
		if (comp->m > 1) {
			struct VertexList* e;
			for (e=comp->edges; e!=NULL; e=e->next) {
				if (occurrences[e->startPoint->number] < compNumber) {
					occurrences[e->startPoint->number] = compNumber;
					++cycleDegrees[e->startPoint->number];
				}
				if (occurrences[e->endPoint->number] < compNumber) {
					occurrences[e->endPoint->number] = compNumber;
					++cycleDegrees[e->endPoint->number];
				}
			}
			++compNumber;
		}			
	}
	free(occurrences);
	return cycleDegrees;
}


int getMaxCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp) {
	int maxDegree = -1;
	int* cycleDegrees = computeCycleDegrees(g, sgp);

	int v;
	for (v=0; v<g->n; ++v) {
		if (maxDegree < cycleDegrees[v]) {
			maxDegree = cycleDegrees[v];
		}
	}

	free(cycleDegrees);

	return maxDegree;
}


int getMinCycleDegree(struct Graph* g, struct ShallowGraphPool* sgp) {
	int minDegree = INT_MAX;
	int* cycleDegrees = computeCycleDegrees(g, sgp);

	int v;
	for (v=0; v<g->n; ++v) {
		if (minDegree > cycleDegrees[v]) {
			minDegree = cycleDegrees[v];
		}
	}

	free(cycleDegrees);

	return minDegree;
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
