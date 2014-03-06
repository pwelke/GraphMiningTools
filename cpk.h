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
 
int CyclicPatternKernel(struct Graph *g, struct ShallowGraphPool *sgp, struct GraphPool *gp, char outputOptions, struct Vertex* globalTreeSet, struct Vertex* globalCycleSet, struct compInfo** results, int* resSize);


#endif /* CPK_H_ */
