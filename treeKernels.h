/*
 * treeKernels.h
 *
 *  Created on: Feb 12, 2012
 *      Author: pascal
 */

#ifndef TREEKERNELS_H_
#define TREEKERNELS_H_

int fullSubtreeEnumeration(struct Graph* t, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct ShallowGraph* bfsSubtreeEnumeration(struct Graph* t, int maxDepth, struct ShallowGraphPool* sgp);
struct ShallowGraph* bfsFreeSubtreeEnumeration(struct Graph* t, int maxDepth, struct ShallowGraphPool* sgp);

#endif /* TREEKERNELS_H_ */
