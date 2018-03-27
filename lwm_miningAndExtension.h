//#include <string.h>
#ifndef LEVELWISE_GRAPH_MINING_H_
#define LEVELWISE_GRAPH_MINING_H_

#include "graph.h"
#include "loading.h"
#include "intSet.h"
#include "subtreeIsoDataStoreList.h"


void filterInfrequentCandidates(// input
		struct Graph* extensions,
		struct SupportSet* supports,
		size_t threshold,
		// output
		struct Graph** filteredExtensions,
		struct SupportSet** filteredSupports,
		// memory management
		struct GraphPool* gp);
struct SupportSet* getCandidateSupportSuperSet(struct IntSet* parentIds, struct SupportSet* previousLevelSupportLists, int parentIdToKeep);
void extendPreviousLevel(// input
		struct SupportSet* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		struct ShallowGraph* extensionEdges,
		size_t threshold,
		// output
		struct SupportSet** resultCandidateSupportSuperSets,
		struct Graph** resultCandidates,
		FILE* logStream,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

struct SupportSet* BFSgetNextLevel(// input
		struct SupportSet* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		size_t threshold,
		struct ShallowGraph* frequentEdges,
		// embedding operator function pointer,
		struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
		double importance,
		// output
		struct Vertex** currentLevelSearchTree,
		FILE* logStream,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

void BFSStrategy(size_t startPatternSize,
					  size_t maxPatternSize,
		              size_t threshold,
					  struct Vertex* initialFrequentPatterns,
					  struct SupportSet* supportSets,
					  struct ShallowGraph* extensionEdges,
					  // embedding operator function pointer,
					  struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
					  double importance,
					  FILE* featureStream,
					  FILE* patternStream,
					  FILE* logStream,
					  struct GraphPool* gp,
					  struct ShallowGraphPool* sgp);

#endif
