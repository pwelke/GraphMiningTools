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

