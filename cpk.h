/*
 * cpk.h
 *
 *  Created on: Jan 18, 2012
 *      Author: pascal
 */

#ifndef CPK_H_
#define CPK_H_

#include "graph.h"
#include "searchTree.h"
 
int CyclicPatternKernel(struct Graph *g, struct ShallowGraphPool *sgp, struct GraphPool *gp,
		char outputOptions, struct Vertex* globalPatternSet, struct compInfo** results, int* resSize);
int CyclicPatternKernel_onlyTrees(struct Graph *g, struct ShallowGraphPool *sgp, struct GraphPool *gp,
		struct Vertex* globalPatternSet, struct compInfo** results, int* resSize);
int CyclicPatternKernel_onlyCycles(struct Graph *g, struct ShallowGraphPool *sgp, struct GraphPool *gp,
		struct Vertex* globalPatternSet, struct compInfo** results, int* resSize);

#endif /* CPK_H_ */
