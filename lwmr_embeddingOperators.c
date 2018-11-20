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

	struct Vertex* rootEmbedding = computeRootedSubtreeEmbedding(data.g, data.g->vertices[0], h, h->vertices[0], gp);
	struct SubtreeIsoDataStore result = data;
	result.foundIso = (rootEmbedding == NULL) ? 0 : 1;
	result.h = h;

	return result;
}

