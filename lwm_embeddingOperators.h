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
struct SubtreeIsoDataStore subtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore relativeImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore absoluteImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore iterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore andorEmbeddingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore localEasySubtreeCheckOperatorWithResampling(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore noniterativeLocalEasySubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore localEasySubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore noniterativeLocalEasySamplingSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore localEasySamplingSubtreeCheckOperatorIndependent(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore alwaysReturnTrue(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore subtreeIsomorphismSamplingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore subtreeIsomorphismSamplingOperatorWithImageShuffling(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore subtreeIsomorphismSamplingOperatorWithMatching(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct SubtreeIsoDataStore subtreeIsomorphismSamplingOperatorWithSampledMatching(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
#endif /* LWM_EMBEDDINGOPERATORS_H_ */
