/*
 * minhashing.h
 *
 *  Created on: May 12, 2016
 *      Author: pascal
 */

#ifndef MINHASHING_H_
#define MINHASHING_H_


#include "graph.h"

struct PosPair {
	size_t level;
	size_t permutation;
};

struct EvaluationPlan {
	struct Graph* poset;
	struct Graph* reversePoset;
	struct PosPair* order;
	int** shrunkPermutations;
	size_t orderLength;
	size_t sketchSize;
};

// PERMUTATIONS
int* getRandomPermutation(int n);
int posetPermutationMark(int* permutation, size_t n, struct Graph* F);
int* posetPermutationShrink(int* permutation, size_t n, size_t shrunkSize);

// EVALUATION PLAN
struct EvaluationPlan buildMinHashEvaluationPlan(int** shrunkPermutations, size_t* permutationSizes, size_t K, struct Graph* F, struct GraphPool* gp);
struct EvaluationPlan dumpEvaluationPlan(struct EvaluationPlan p, struct GraphPool* gp);
void cleanEvaluationPlan(struct EvaluationPlan p);

// BUILD TREE POSET
struct Graph* buildTreePosetFromGraphDB(struct Graph** db, int nGraphs, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct Graph* reverseGraph(struct Graph* g, struct GraphPool* gp);
void updateEvaluationPlan(struct EvaluationPlan p, int patternId, char match);

int embeddingValueKnown(struct Vertex* v);
int getMatch(struct Vertex* v);
struct IntSet* patternPosetInfoToFeatureSet(struct EvaluationPlan p);

// COMPUTATION OF MINHASH EMBEDDINGS
int* fastMinHashForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp);
int* fastMinHashForRelImportantTrees(struct Graph* g, struct EvaluationPlan p, double importance, struct GraphPool* gp);
int* fastMinHashForAbsImportantTrees(struct Graph* g, struct EvaluationPlan p, int importance, struct GraphPool* gp);
int* fastMinHashForAndOr(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp);

// STANDARD EMBEDDING APPROXIMATION AND RANDOM PROJECTION EMBEDDINGS
int* fullEmbeddingProjectionApproximationForTrees(struct Graph* g, struct EvaluationPlan p, int* projection, int projectionSize, struct GraphPool* gp);
int* fullEmbeddingProjectionApproximationLocalEasy(struct Graph* g, struct EvaluationPlan p, int* projection, int projectionSize, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp);
int* randomProjectionEmbeddingForTrees(struct Graph* g, struct EvaluationPlan p, int* projection, int projectionSize, struct GraphPool* gp);
int* randomProjectionEmbeddingLocalEasy(struct Graph* g, struct EvaluationPlan p, int* projection, int projectionSize, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp);

// FOR COMPARISON: EXACT COMPUTATION OF EMBEDDING	 USING THE PATTERN POSET
struct IntSet* explicitEmbeddingForTrees(struct Graph* g, struct Graph* F, struct GraphPool* gp, struct ShallowGraphPool* sgp);\
struct IntSet* explicitEmbeddingForAbsImportantTrees(struct Graph* g, struct Graph* F, size_t importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IntSet* explicitEmbeddingForRelImportantTrees(struct Graph* g, struct Graph* F, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
//struct IntSet* explicitEmbeddingForLocalEasyOperator(struct Graph* g, struct Graph* F, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IntSet* explicitEmbeddingForLocalEasyOperator(struct Graph* g, struct EvaluationPlan p, int nLocalTrees, struct GraphPool* gp, struct ShallowGraphPool* sgp);


#endif /* MINHASHING_H_ */
