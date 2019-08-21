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
struct SubtreeIsoDataStore subtreeOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore subtreeRelimpOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore subtreeAbsimpOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore subtreeIterativeOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore andorEmbeddingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore localEasySubtreeCheckOperatorWithResampling(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore noniterativeLocalEasySubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore localEasyOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore noniterativeLocalEasySamplingSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore alwaysReturnTrue(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore hopsSimpleOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore hopsSimplerandomOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore hopsSimplematchingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore hopsOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
#endif /* LWM_EMBEDDINGOPERATORS_H_ */
