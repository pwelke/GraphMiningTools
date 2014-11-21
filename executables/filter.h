#ifndef FILTER_H_
#define FILTER_H_

typedef enum {
	leq,
	eq,
	geq,
	neq,
	less,
	greater
} Comparator;

char conditionHolds(int measure, int threshold, Comparator comparator);

typedef enum {
	graph,
	idAndValue,
	id,
	value
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

	/* TODO additional Parameter needed*/
	spanningTreeListing
} Filter;

char isTree(struct Graph* g);
char isCactus(struct Graph* g, struct ShallowGraphPool* sgp);
char isOuterplanarGraph(struct Graph* g, struct ShallowGraphPool* sgp, struct GraphPool* gp);

int getNumberOfBridges(struct Graph* g, struct ShallowGraphPool* sgp);
int getNumberOfBlocks(struct Graph* g, struct ShallowGraphPool* sgp);

void processGraph(int i, struct Graph* g, Filter filter, Comparator comparator, int value, FILE* out, OutputOption oOption, struct ShallowGraphPool* sgp, struct GraphPool* gp);
void output(struct Graph* g, int measure, OutputOption option, FILE* out);

#endif