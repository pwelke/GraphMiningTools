/*
 * lwm_embeddingOperators.c
 *
 *  Created on: Oct 28, 2016
 *      Author: pascal
 */

#ifndef LWMR_EMBEDDINGOPERATORS_H_
#define LWMR_EMBEDDINGOPERATORS_H_

#include "graph.h"
#include "newCube.h" // for SubtreeIsoDataStore

struct SubtreeIsoDataStore rootedSubtreeComputationOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore rootedHopsOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif /* LWMR_EMBEDDINGOPERATORS_H_ */

