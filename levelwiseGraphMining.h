//#include <string.h>

#include "graph.h"
#include "loading.h"
#include "intSet.h"
#include "subtreeIsoDataStoreList.h"

int getDB(struct Graph*** db);
int getDBfromCanonicalStrings(struct Graph*** db, FILE* stream, int bufferSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int getFrequentVertices(struct Graph** db, int dbSize, struct Vertex* frequentVertices, struct GraphPool* gp);
void getFrequentEdges(struct Graph** db, int dbSize, int initialResultSetSize, struct Vertex* frequentEdges, struct GraphPool* gp);

void stupidPatternEvaluation(struct Graph** db, int nGraphs, struct Graph** patterns, int nPatterns, struct Vertex** pointers, struct GraphPool* gp);
void DFS(struct Graph** db, struct IntSet* candidateSupport, struct Graph* candidate, size_t threshold, int maxPatternSize, struct ShallowGraph* frequentEdges, struct Vertex* processedPatterns, FILE* featureStream, FILE* patternStream, FILE* logStream, struct GraphPool* gp, struct ShallowGraphPool* sgp);
void iterativeDFS(struct SubtreeIsoDataStoreList* candidateSupport,
		size_t threshold,
		int maxPatternSize,
		struct ShallowGraph* frequentEdges,
		struct Vertex* processedPatterns,
	    // embedding operator function pointer,
	    struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*),
	    double importance,
		FILE* featureStream,
		FILE* patternStream,
		FILE* logStream,
		struct GraphPool* gp,
		struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStoreList* initIterativeDFS(struct Graph** db, size_t nGraphs, struct VertexList* e, int edgeId, struct GraphPool* gp);
void iterativeDFSMain(size_t maxPatternSize,
		              size_t threshold,
					  // embedding operator function pointer,
					 struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
					 double importance,
					 FILE* featureStream,
					 FILE* patternStream,
					 FILE* logStream,
					 struct GraphPool* gp,
					 struct ShallowGraphPool* sgp);
void DFSMain(size_t maxPatternSize, size_t threshold, FILE* featureStream, FILE* patternStream, FILE* logStream, struct GraphPool* gp, struct ShallowGraphPool* sgp);
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
struct SubtreeIsoDataStoreList* iterativeBFSOneLevel(// input
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

struct SubtreeIsoDataStoreList* initIterativeBFSForEdges(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId);
struct SubtreeIsoDataStoreList* initIterativeBFSForVertices(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId);
int** getPostorders(struct Graph** db, int nGraphs);
void getFrequentVerticesAndEdges(struct Graph** db, int nGraphs, size_t threshold, struct Vertex** frequentVertices, struct Vertex** frequentEdges, FILE* logStream, struct GraphPool* gp);
struct SubtreeIsoDataStoreList* initLevelwiseMining(struct Graph** db, int** postorders, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp);

void madness(struct SubtreeIsoDataStoreList* previousLevelSupportSets, struct Vertex* previousLevelSearchTree,
		     struct ShallowGraph* extensionEdges, size_t maxPatternSize, size_t threshold,
			 struct SubtreeIsoDataStoreList** currentLevelSupportSets, struct Vertex** currentLevelSearchTree,
			 FILE* featureStream, FILE* patternStream, FILE* logStream,
			 struct GraphPool* gp, struct ShallowGraphPool* sgp);
void iterativeBFSMain(size_t maxPatternSize,
		              size_t threshold,
					  // embedding operator function pointer,
					  struct SubtreeIsoDataStore (*embeddingOperator)(struct SubtreeIsoDataStore, struct Graph*, double, struct GraphPool*, struct ShallowGraphPool*),
					  double importance,
					  FILE* featureStream,
					  FILE* patternStream,
					  FILE* logStream,
					  struct GraphPool* gp,
					  struct ShallowGraphPool* sgp);

struct SubtreeIsoDataStore noniterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore relativeImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore absoluteImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore iterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore andorEmbeddingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore noniterativeLocalEasySamplingSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
