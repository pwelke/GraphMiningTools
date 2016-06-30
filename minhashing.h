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
	struct Graph* F;
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
struct EvaluationPlan buildEvaluationPlan(int** shrunkPermutations, size_t* permutationSizes, size_t K, struct Graph* F);
struct EvaluationPlan dumpEvaluationPlan(struct EvaluationPlan p, struct GraphPool* gp);

// BUILD TREE POSET
struct Graph* buildTreePosetFromGraphDB(struct Graph** db, int nGraphs, struct GraphPool* gp, struct ShallowGraphPool* sgp);

// COMPUTATION OF MINHASHES
int* fastMinHashForTrees(struct Graph* g, struct EvaluationPlan p, struct GraphPool* gp);
int* fastMinHashForRelImportantTrees(struct Graph* g, struct EvaluationPlan p, double importance, struct GraphPool* gp);
int* fastMinHashForAbsImportantTrees(struct Graph* g, struct EvaluationPlan p, int importance, struct GraphPool* gp);

// FOR COMPARISON: EXPLICIT EVALUATION USING THE PATTERN POSET
struct IntSet* explicitEmbeddingForTrees(struct Graph* g, struct Graph* F, struct GraphPool* gp, struct ShallowGraphPool* sgp);\
struct IntSet* explicitEmbeddingForAbsImportantTrees(struct Graph* g, struct Graph* F, size_t importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);
struct IntSet* explicitEmbeddingForRelImportantTrees(struct Graph* g, struct Graph* F, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp);

#endif /* MINHASHING_H_ */
