/*
 * centerOfTree.h
 *
 *  Created on: May 7, 2012
 *      Author: pascal
 */

#ifndef CENTEROFTREE_H_
#define CENTEROFTREE_H_

struct ShallowGraph* getTreeCenter(struct Vertex* v, int maxDepth, struct ShallowGraphPool* sgp);
void printList(struct ShallowGraph* currentLeaves);

#endif /* CENTEROFTREE_H_ */
