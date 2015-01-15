#ifndef FILTER_H_
#define FILTER_H_

typedef enum {
	leq,
	eq,
	geq,
	neq,
	less,
	greater,
	pass
} Comparator;

char conditionHolds(int measure, int threshold, Comparator comparator);

typedef enum {
	graph,
	idAndValue,
	onlyValue,
	id
} OutputOption;

typedef enum {
	/* counting */
	graphName,
	count,

	/* labels */
	label,
	AvsI,
	AMvsI,
	AvsMI,

	/* boolean properties */
	connected,
	outerplanar,
	tree,
	cactus,

	/* numerical properties */
	spanningTreeEstimate,
	numberOfBlocks,
	numberOfBridges,
	numberOfBridgeTrees,
	numberOfSimpleCycles,
	numberOfNonIsoCycles,
	numberOfConnectedComponents,
	numberOfVertices,
	numberOfEdges,
	maxCycleDegree,
	minCycleDegree,
	maxDegree,
	minDegree,

	/* TODO additional Parameter needed*/
	spanningTreeListing
} Filter;

void processGraph(int i, struct Graph* g, Filter filter, Comparator comparator, int value, FILE* out, OutputOption oOption, struct ShallowGraphPool* sgp, struct GraphPool* gp);
void output(struct Graph* g, int measure, OutputOption option, FILE* out);

#endif