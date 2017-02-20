/*
 * poset_pathCover.h
 *
 *  Created on: Feb 17, 2017
 *      Author: pascal
 */

#ifndef POSET_PATHCOVER_H_
#define POSET_PATHCOVER_H_

int numberOfReachableVertices(struct Vertex* v, struct ShallowGraphPool* sgp);
void computeAllReachabilityCounts(struct Graph* g, struct ShallowGraphPool* sgp);
int** getPathCoverOfPoset(struct Graph* g, size_t* nPaths, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif /* POSET_PATHCOVER_H_ */
