/*
 * poset_pathCover.h
 *
 *  Created on: Feb 17, 2017
 *      Author: pascal
 */

#ifndef POSET_PATHCOVER_H_
#define POSET_PATHCOVER_H_

#include "graph.h"
#include "minhashing.h"

int numberOfReachableVertices(struct Vertex* v, struct ShallowGraphPool* sgp);
void computeAllReachabilityCounts(struct Graph* g, struct ShallowGraphPool* sgp);
int** getPathCoverOfPoset(struct Graph* g, size_t* nPaths, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int** getPathCoverOfPosetPR(struct Graph* g, size_t* nPaths, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int checkPathCover(struct Graph* g, int** pathset, size_t nPaths);


// FAST COMPUTATION OF EXACT EMBEDDINGS
struct IntSet* dfsDownwardEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp);
struct IntSet* latticePathEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp);
struct IntSet* latticeLongestPathEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IntSet* staticPathCoverEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp);
struct IntSet* latticeLongestWeightedPathEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, int databaseSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);


struct IntSet* dfsDownwardEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IntSet* latticePathEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IntSet* latticeLongestPathEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IntSet* staticPathCoverEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IntSet* latticeLongestWeightedPathEmbeddingForLocalEasy(struct Graph* g, struct EvaluationPlan p, int sampleSize, int databaseSize, struct GraphPool* gp, struct ShallowGraphPool* sgp);
#endif /* POSET_PATHCOVER_H_ */
