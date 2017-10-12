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

	/* boolean properties */
	connected,
	outerplanar,
	tree,
	path,
	cactus,
	traceableCactus,
	weaklyTraceable,

	/* numerical properties */
	spanningTreeEstimate,
	spanningTreeListing,
	nonisomorphicSpanningTrees,
	nonisomorphicSampledSpanningTrees,
	nonisomorphicLocallySampledSpanningTrees,
	nonisomorphicLocallySampledSpanningTreesFiltered,
	locallySampledSpanningTrees,
	locallySampledSpanningTreesFiltered,
	maxBlocksPerComponent,
	numberOfBlocks,
	numberOfBridges,
	numberOfBridgeTrees,
	numberOfSimpleCycles,
	numberOfNonIsoCycles,
	numberOfBiconnectedComponents,
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
