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
#include "../listCycles.h"
#include "../cs_Cycle.h"
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
			if (strcmp(optarg, "numberOfBridgeTrees") == 0) {
				filter = numberOfBridgeTrees;
				break;
			}
			if (strcmp(optarg, "numberOfSimpleCycles") == 0) {
				filter = numberOfSimpleCycles;
				break;
			}
			if (strcmp(optarg, "numberOfNonIsoCycles") == 0) {
				filter = numberOfNonIsoCycles;
				break;
			}
			if (strcmp(optarg, "numberOfVertices") == 0) {
				filter = numberOfVertices;
				break;
			}
			if (strcmp(optarg, "numberOfConnectedComponents") == 0) {
				filter = numberOfConnectedComponents;
				break;
			}
			if (strcmp(optarg, "numberOfEdges") == 0) {
				filter = numberOfEdges;
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
				oOption = onlyValue;
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
		measure = i;
		break;
	case graphName:
		measure = g->number;
		break;

	/* labels */
	case label:
		measure = g->activity;
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
		measure = g->activity;
		break;
	case AvsMI:
		if (g->activity == 2) {
			g->activity = 1;
		} else {
			g->activity = -1;
		}
		measure = g->activity;
		break;
	case AMvsI:
		if (g->activity == 0) {
			g->activity = -1;
		}
		if (g->activity == 2) {
			g->activity = 1;
		}
		measure = g->activity;
		break;

	/* boolean properties */
	case connected:
		measure = isConnected(g);
		break;
	case outerplanar:
		// isMaximalOuterplanar alters g. do not attempt to do something with g after this.
		measure = isOuterplanarGraph(g, sgp, gp);
		break;
	case tree:
		measure = isTree(g);
		break;
	case cactus:
		measure = isCactus(g, sgp);
		break;

	/* numeric properties */
	case spanningTreeEstimate:
		measure = getGoodEstimate(g, sgp, gp);
		break;
	case numberOfBlocks:
		measure = getNumberOfBlocks(g, sgp);			
		break;
	case numberOfBridges:
		measure = getNumberOfBridges(g, sgp);			
		break;
	case numberOfBridgeTrees:
		measure = getNumberOfBridgeTrees(g, sgp, gp);			
		break;
	case numberOfVertices:
		measure = g->n;
		break;
	case numberOfEdges:
		measure = g->m;
		break;
	case numberOfNonIsoCycles:
		measure = getNumberOfNonIsoCycles(g, sgp, gp);
		break;
	case numberOfSimpleCycles:
		measure = getNumberOfSimpleCycles(g, sgp, gp);
		break;
		
	case numberOfConnectedComponents:
		measure = listConnectedComponents(g);
		break;
	case maxDegree:
		measure = getMaxDegree(g);
		break;
	case minDegree:
		measure = getMinDegree(g);
		break;
	case maxCycleDegree:
		measure = getMaxCycleDegree(g, sgp);
		break;
	case minCycleDegree:
		measure = getMinCycleDegree(g, sgp);
		break;
	}

	if (conditionHolds(measure, value, comparator)) {
		output(g, measure, oOption, out);
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
	case onlyValue:
		fprintf(out, "%i\n", measure);
		break;
	case idAndValue:
		fprintf(out, "%i %i\n", g->number, measure);
		break;
	case id:
		fprintf(out, "%i\n", g->number);
		break;
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
Count the number of connected components in the graph obtained from g by removing
all block edges (i.e. all edges that are not bridges)
*/
int getNumberOfBridgeTrees(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	int nConnectedComponents = listConnectedComponents(forest);
	dumpGraphList(gp, forest);
	return nConnectedComponents;
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


int getNumberOfSimpleCycles(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct Graph* tmp;
	struct Graph* idx;
	int numCycles = 0;

	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	/* TODO refactor */
	struct Graph* biconnectedComponents = forest->next;

	/* list all cycles */
	struct ShallowGraph* simpleCycles = NULL;


	for (idx=biconnectedComponents; idx; idx=idx->next) {
		simpleCycles = addComponent(simpleCycles, listCycles(idx, sgp));
	}

	/* if cycles were found, compute canonical strings */
	if (simpleCycles) {
		struct ShallowGraph* cycle = NULL;

		/* transform cycle of shallow graphs to a list */
		simpleCycles->prev->next = NULL;
		simpleCycles->prev = NULL;

		for (cycle=simpleCycles; cycle!=NULL; cycle=cycle->next) {
			++numCycles;
		}

		dumpShallowGraphCycle(sgp, simpleCycles);
	} 

	/* garbage collection */
	for (idx=biconnectedComponents; idx; idx=tmp) {
		tmp = idx->next;
		dumpGraph(gp, idx);
	}
	dumpGraph(gp, forest);

	/* each cycle is found twice */
	return numCycles / 2;
}

int getNumberOfNonIsoCycles(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp) {
	struct Graph* tmp;
	struct Graph* idx;
	int numCycles;

	/* find biconnected Components */
	struct ShallowGraph* h = listBiconnectedComponents(g, sgp);
	struct Graph* forest = partitionIntoForestAndCycles(h, g, gp, sgp);
	/* TODO refactor */
	struct Graph* biconnectedComponents = forest->next;

	/* list all cycles */
	struct ShallowGraph* simpleCycles = NULL;
	struct ShallowGraph* cyclePatterns = NULL;
	struct Vertex* cyclePatternSearchTree = NULL;

	for (idx=biconnectedComponents; idx; idx=idx->next) {
		simpleCycles = addComponent(simpleCycles, listCycles(idx, sgp));
	}

	/* if cycles were found, compute canonical strings */
	if (simpleCycles) {
		/* transform cycle of shallow graphs to a list */
		simpleCycles->prev->next = NULL;
		simpleCycles->prev = NULL;

		cyclePatterns = getCyclePatterns(simpleCycles, sgp);
		cyclePatternSearchTree = buildSearchTree(cyclePatterns, gp, sgp);

		numCycles = cyclePatternSearchTree->number;
	} else {
		numCycles = 0;
	}

	/* garbage collection */

	/* dump cycles, if any
	 * TODO may be moved upwards directly after finding simple cycles */
	if (cyclePatternSearchTree) {
			dumpSearchTree(gp, cyclePatternSearchTree);
	}

	/* dump biconnected components list */
	for (idx=biconnectedComponents; idx; idx=tmp) {
		tmp = idx->next;
		dumpGraph(gp, idx);
	}

	dumpGraph(gp, forest);

	/* each cycle is found twice */
	return numCycles / 2;
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
