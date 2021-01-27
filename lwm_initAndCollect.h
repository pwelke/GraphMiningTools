/*
 * lwm_initAndCollect.h
 *
 *  Created on: Mar 27, 2018
 *      Author: pascal
 */

#ifndef LWM_INITANDCOLLECT_H_
#define LWM_INITANDCOLLECT_H_

#include "supportSet.h"

//int** getPostorders(struct Graph** db, int nGraphs);
int getDB(struct Graph*** db);
int getDBfromCanonicalStrings(struct Graph*** db, FILE* stream, size_t bufferSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);
// should be static, but are required by lwmr_initAndCollect
int getFrequentVertices(struct Graph** db, int dbSize, struct Vertex* frequentVertices, struct GraphPool* gp);
struct ShallowGraph* edgeSearchTree2ShallowGraph(struct Vertex* frequentEdges, struct Graph** newVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SupportSet* getSupportSetsOfVerticesForPatternEnumeration(struct Graph** db, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IterativeBfsForForestsDataStructures {
	struct Graph** db;
	int** postorders;
	struct Vertex* initialFrequentPatterns;
	struct ShallowGraph* extensionEdges;
	struct Graph* extensionEdgesVertexStore;
	int nGraphs;
};


//struct SupportSet* getSupportSetsOfVertices(struct Graph** db, int** postoderDB, size_t nGraphs, struct Graph* h, int patternId);
//void getFrequentVerticesAndEdges(struct Graph** db, int nGraphs, size_t threshold, struct Vertex** frequentVertices, struct Vertex** frequentEdges, FILE* logStream, struct GraphPool* gp);
//struct SupportSet* createSingletonPatternSupportSetsForForestDB(struct Graph** db, int** postorders, int nGraphs, struct Vertex* frequentVertices, struct GraphPool* gp, struct ShallowGraphPool* sgp);

// inits and garbage collectors
size_t initFrequentTreeMiningForForestDB(// input
		size_t threshold,
		double importance,
		// output
		struct Vertex** initialFrequentPatterns,
		struct SupportSet** supportSets,
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
		struct SupportSet** supportSets,
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
		struct SupportSet** supportSets,
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
		struct SupportSet** supportSets,
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
		struct SupportSet** supportSets,
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
		struct SupportSet** supportSets,
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
void garbageCollectPatternEnumeration(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp);



#endif /* LWM_INITANDCOLLECT_H_ */
