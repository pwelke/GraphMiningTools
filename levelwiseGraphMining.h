//#include <string.h>
#ifndef LEVELWISE_GRAPH_MINING_H_
#define LEVELWISE_GRAPH_MINING_H_

#include "graph.h"
#include "loading.h"
#include "intSet.h"
#include "subtreeIsoDataStoreList.h"

int** getPostorders(struct Graph** db, int nGraphs);
int getDB(struct Graph*** db);
int getDBfromCanonicalStrings(struct Graph*** db, FILE* stream, int bufferSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int getFrequentVertices(struct Graph** db, int dbSize, struct Vertex* frequentVertices, struct GraphPool* gp);
void getFrequentEdges(struct Graph** db, int dbSize, int initialResultSetSize, struct Vertex* frequentEdges, struct GraphPool* gp);

void filterInfrequentCandidates(// input
		struct Graph* extensions,
		struct SubtreeIsoDataStoreList* supports,
		size_t threshold,
		// output
		struct Graph** filteredExtensions,
		struct SubtreeIsoDataStoreList** filteredSupports,
		// memory management
		struct GraphPool* gp);
struct SubtreeIsoDataStoreList* getCandidateSupportSuperSet(struct IntSet* parentIds, struct SubtreeIsoDataStoreList* previousLevelSupportLists, int parentIdToKeep);
void extendPreviousLevel(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
		struct Vertex* previousLevelSearchTree,
		struct ShallowGraph* extensionEdges,
		size_t threshold,
		// output
		struct SubtreeIsoDataStoreList** resultCandidateSupportSuperSets,
		struct Graph** resultCandidates,
		FILE* logStream,
		// memory management
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

struct SubtreeIsoDataStoreList* BFSgetNextLevel(// input
		struct SubtreeIsoDataStoreList* previousLevelSupportLists,
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

struct SubtreeIsoDataStoreList* getSupportSetsOfVertices(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId);
void getFrequentVerticesAndEdges(struct Graph** db, int nGraphs, size_t threshold, struct Vertex** frequentVertices, struct Vertex** frequentEdges, FILE* logStream, struct GraphPool* gp);
struct SubtreeIsoDataStoreList* createSingletonPatternSupportSetsForForestDB(struct Graph** db, int** postorders, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp);

void BFSStrategy(size_t startPatternSize,
					  size_t maxPatternSize,
		              size_t threshold,
					  struct Vertex* initialFrequentPatterns,
					  struct SubtreeIsoDataStoreList* supportSets,
					  struct ShallowGraph* extensionEdges,
					  // embedding operator function pointer,
					  struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
					  double importance,
					  FILE* featureStream,
					  FILE* patternStream,
					  FILE* logStream,
					  struct GraphPool* gp,
					  struct ShallowGraphPool* sgp);

size_t initFrequentTreeMiningForForestDB(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

size_t initProbabilisticTreeMiningForGraphDB(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

size_t initGlobalTreeEnumerationForGraphDB(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

size_t initExactLocalEasyForGraphDB(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

size_t initSampledLocalEasyForGraphDB(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

size_t initSampledLocalEasyWithDuplicatesForGraphDB(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

size_t initPatternEnumeration(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SubtreeIsoDataStoreList** supportSets,
		struct ShallowGraph** extensionEdgeList,
		void** dataStructures,
		// printing
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		// pools
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);

void garbageCollectFrequentTreeMiningForForestDB(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void garbageCollectLocalEasyForGraphDB(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void garbagePatternEnumeration(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif
