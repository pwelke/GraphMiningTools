/*
 * lwm_embeddingOperators.c
 *
 *  Created on: Oct 28, 2016
 *      Author: pascal
 */

#include <stddef.h>

#include "subtreeIsoUtils.h"
#include "subtreeIsomorphism.h"
#include "iterativeSubtreeIsomorphism.h"
#include "importantSubtrees.h"
#include "localEasySubtreeIsomorphism.h"

#include "lwm_embeddingOperators.h"


void stupidPatternEvaluation(struct Graph** db, int nGraphs, struct Graph** patterns, int nPatterns, struct Vertex** pointers, struct GraphPool* gp) {
	int i;
	for (i=0; i<nGraphs; ++i) {
		int j;
		for (j=0; j<nPatterns; ++j) {
			if (subtreeCheck3(db[i], patterns[j], gp)) {
				++pointers[j]->visited;
			}
		}
	}
}



// WRAPPERS FOR DIFFERENT EMBEDDING OPERATORS

struct SubtreeIsoDataStore subtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	return noniterativeSubtreeCheck(data, h, gp);
}


struct SubtreeIsoDataStore iterativeSubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	return iterativeSubtreeCheck(data, h, gp);
}


struct SubtreeIsoDataStore noniterativeLocalEasySamplingSubtreeCheckOperatorWithResampling(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;

	result.foundIso = isProbabilisticLocalSampleSubtree(result.g, result.h, (int)importance, gp, sgp);
	return result;
}


struct SubtreeIsoDataStore localEasySubtreeCheckOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)importance; // unused
	(void)sgp; // unused

	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	struct SpanningtreeTree* sptTree = (struct SpanningtreeTree*)data.postorder;

	result.foundIso = subtreeCheckForSpanningtreeTree(sptTree, h, gp);

	// clean up the spanning tree tree
	wipeCharacteristicsForLocalEasy(*sptTree);

	return result;
}


struct SubtreeIsoDataStore relativeImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = isImportantSubtreeRelative(result.g, result.h, importance, gp);
	return result;
}


struct SubtreeIsoDataStore absoluteImportanceOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = isImportantSubtreeAbsolute(result.g, result.h, importance, gp);
	return result;
}


struct SubtreeIsoDataStore andorEmbeddingOperator(struct SubtreeIsoDataStore data, struct Graph* h, double importance, struct GraphPool* gp, struct ShallowGraphPool* sgp) {
	(void)sgp; // unused
	(void)importance; // unused
	struct SubtreeIsoDataStore result = data;
	result.h = h;
	result.S = NULL;
	result.foundIso = andorEmbedding(result.g, result.h, gp);
	return result;
}
