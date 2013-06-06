/*
 * opk.h
 *
 *  Created on: Feb 12, 2012
 *      Author: pascal
 */

#ifndef OPK_H_
#define OPK_H_

void outerplanarKernel(struct Graph *g, int depth, struct ShallowGraphPool *sgp, struct GraphPool *gp, char outputOptions, struct Vertex* globalTreeSet, struct compInfo** results, int* resSize);
void freeOuterplanarKernel(struct Graph *g, int depth, struct ShallowGraphPool *sgp, struct GraphPool *gp, char outputOptions, struct Vertex* globalTreeSet, struct compInfo** results, int* resSize);

#endif /* OPK_H_ */
