/*
 * lwm_embeddingOperators.c
 *
 *  Created on: Oct 28, 2016
 *      Author: pascal
 */

#include <stddef.h>
#include <stdlib.h>

#include "subtreeIsoUtils.h"
#include "subtreeIsomorphism.h"
#include "iterativeSubtreeIsomorphism.h"
#include "importantSubtrees.h"
#include "localEasySubtreeIsomorphism.h"
#include "subtreeIsomorphismSampling.h"

#include "lwm_embeddingOperators.h"






// WRAPPERS FOR DIFFERENT EMBEDDING OPERATORS

/**
 * Exact embedding Operator for
 * tree pattern h
 * forest transaction data
 */
struct SubtreeIsoDataStore subtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	// TODO
//	return noniterativeSubtreeCheck(data, h, gp);
}

