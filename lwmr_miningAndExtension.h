//#include <string.h>
#ifndef LEVELWISE_GRAPH_MINING_H_
#define LEVELWISE_GRAPH_MINING_H_

#include "graph.h"
#include "loading.h"
#include "intSet.h"
#include "supportSet.h"

void BFSStrategyRooted(size_t startPatternSize,
					   size_t maxPatternSize,
					   size_t threshold,
					   struct Vertex *initialFrequentPatterns,
					   struct SupportSet *supportSets,
					   struct ShallowGraph *extensionEdges,
					   // embedding operator function pointer,
					   struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph *, double, struct GraphPool *, struct ShallowGraphPool *),
					   double importance,
					   FILE *featureStream,
					   FILE *patternStream,
					   FILE *logStream,
					   struct GraphPool *gp,
					   struct ShallowGraphPool *sgp);

	void DFSStrategyRooted(size_t startPatternSize,
						   size_t maxPatternSize,
						   size_t threshold,
						   struct Vertex *initialFrequentPatterns,
						   struct SupportSet *supportSets,
						   struct ShallowGraph *extensionEdges,
						   // embedding operator function pointer,
						   struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph *, double, struct GraphPool *, struct ShallowGraphPool *),
						   double importance,
						   FILE *featureStream,
						   FILE *patternStream,
						   FILE *logStream,
						   struct GraphPool *gp,
						   struct ShallowGraphPool *sgp);

#endif
