/*
 * lwm_embeddingOperators.h
 *
 *  Created on: Oct 28, 2016
 *      Author: pascal
 */

#ifndef LWM_EMBEDDINGOPERATORS_H_
#define LWM_EMBEDDINGOPERATORS_H_

#include "graph.h"
#include "newCube.h" // for SubtreeIsoDataStore


void stupidPatternEvaluation(struct Graph** db, int nGraphs, struct Graph** patterns, int nPatterns, struct Vertex** pointers, struct GraphPool* gp);


// embedding operators
struct SubtreeIsoDataStore noniterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore relativeImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore absoluteImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore iterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore andorEmbeddingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore noniterativeLocalEasySamplingSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore noniterativeLocalEasySubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);


#endif /* LWM_EMBEDDINGOPERATORS_H_ */
