/*
 * lwm_initAndCollect.h
 *
 *  Created on: Mar 27, 2018
 *      Author: pascal
 */

#ifndef LWMR_INITANDCOLLECT_H_
#define LWMR_INITANDCOLLECT_H_

#include "supportSet.h"

//int** getPostorders(struct Graph** db, int nGraphs);
int getDirectedDB(struct Graph*** db);

size_t initDirectedPatternEnumeration(// input
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

void garbageCollectDirectedPatternEnumeration(void** y, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif /* LWM_INITANDCOLLECT_H_ */
