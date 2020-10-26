/*
 * lwm_embeddingOperators.c
 *
 *  Created on: Oct 28, 2016
 *      Author: pascal
 */

#include <stddef.h>
#include <stdlib.h>

#include "subtreeIsoUtils.h"
#include "iterativeSubtreeIsomorphism.h"
#include "importantSubtrees.h"
#include "localEasySubtreeIsomorphism.h"
#include "subtreeIsomorphismSampling.h"

#include "lwm_embeddingOperators.h"






// WRAPPERS FOR DIFFERENT ROOTED TREE EMBEDDING OPERATORS

/**
 * Exact embedding Operator for
 * tree pattern h
 * forest transaction data
 */
struct SubtreeIsoDataStore rootedSubtreeComputationOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused

	struct Vertex* rootEmbedding = NULL;
	struct SubtreeIsoDataStore result = noniterativeRootedSubtreeCheck(data, h, &rootEmbedding, gp);

//	if (result.foundIso) {
//		// nasty hack to output information on embeddings found (for all patterns, not only frequent ones) to stdout
//		fprintf(stdout, "emb %i %i %i\n", h->number, data.g->number, rootEmbedding->number);
//	}

	return result;
}



/**
 * Randomized embedding operator for
 * tree pattern h
 * graph transaction data
 *
 * The algorithm repeats to try to embed a randomly rooted shuffled version of the tree h
 * in some random place in the transaction graph. If it succeeds at some point, it returns 1.
 * This is another example of an embedding operator with one-sided error.
 *
 * This variant sorts the neighbors of pattern vertex and transaction vertex by label and
 * shuffles the blocks locally to obtain a maximum matching that is sampled uniformly at random
 * from the set of all maximal matchings.
 *
 */
struct SubtreeIsoDataStore rootedHopsOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused

	struct SubtreeIsoDataStore result = {0};
	result.g = data.g;
	result.h = h;

	for (int i=0; i<importance; ++i) {
		result.foundIso = subtreeIsomorphismSamplerWithSampledMaximumMatching(data.g, h, gp, 0);
		if (result.foundIso) {
			break;
		}
	}

	return result;
}
