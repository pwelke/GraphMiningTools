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
	id,
	printVerbose
} OutputOption;

typedef enum {
	/* counting */
	graphName,
	count,
	randomSample,

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
	traceableCactus,
	weaklyTraceable,

	/* numerical properties */
	spanningTreeEstimate,
	spanningTreeListing,
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
	maxLocalEasiness,
	minLocalEasiness,
	maxDegree,
	minDegree

} Filter;

void processGraph(int i, struct Graph* g, Filter filter, Comparator comparator, int value, int additionalParameter, FILE* out, OutputOption oOption, struct ShallowGraphPool* sgp, struct GraphPool* gp);
void output(struct Graph* g, int measure, OutputOption option, FILE* out);

#endif