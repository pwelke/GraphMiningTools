/*
 * subtreeIsoUtils.h
 *
 *  Created on: Apr 18, 2016
 *      Author: pascal
 */

#ifndef SUBTREEISOUTILS_H_
#define SUBTREEISOUTILS_H_

/*
 * subtreeIsoUtils.c
 *
 *  Created on: Apr 18, 2016
 *      Author: pascal
 */

#include "graph.h"

int labelCmp(const char* l1, const char* l2);
int* getPostorder(struct Graph* g, int root);
int* getPostorderForTree(struct Graph* g, int root);
void markReachable(struct Vertex* a, int num);

#endif /* SUBTREEISOUTILS_H_ */
